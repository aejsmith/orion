/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Rigid body component.
 *
 * TODO:
 *  - Kinematic bodies. These need you to handle the effect of collisions
 *    yourself.
 */

#include "physics_priv.h"

#include "engine/world.h"

#include "physics/collision_shape.h"
#include "physics/physics_manager.h"
#include "physics/physics_world.h"
#include "physics/rigid_body.h"

/** Synchronises Bullet and internal states. */
class RigidBody::MotionState : public btMotionState {
public:
    /** Construct the motion state.
     * @param rigidBody     Rigid body that the state is for. */
    explicit MotionState(RigidBody *rigidBody) : m_rigidBody(rigidBody) {}

    /** Get the transformation of the world object.
     * @param transform     Transformation to fill in. */
    void getWorldTransform(btTransform &transform) const {
        transform.setRotation(BulletUtil::toBullet(m_rigidBody->orientation()));
        transform.setOrigin(BulletUtil::toBullet(m_rigidBody->position()));
    }

    /** Set the transformation of the entity in the world.
     * @param transform     New world transformation to use. */
    void setWorldTransform(const btTransform &transform) {
        m_rigidBody->m_updatingTransform = true;
        m_rigidBody->entity()->setOrientation(BulletUtil::fromBullet(transform.getRotation()));
        m_rigidBody->entity()->setPosition(BulletUtil::fromBullet(transform.getOrigin()));
        m_rigidBody->m_updatingTransform = false;
    }
private:
    RigidBody *m_rigidBody;         /**< Rigid body that the state is for. */
};

/**
 * Initialise the rigid body.
 *
 * Initialises the rigid body to default properties: a mass of 0 (a static body),
 * and linear/angular damping factors as 0, and the default physics material as
 * the material.
 *
 * @param entity        Entity that the component belongs to.
 */
RigidBody::RigidBody(Entity *entity) :
    Component(Component::kRigidBodyType, entity),
    m_mass(0.0f),
    m_linearDamping(0.0f),
    m_angularDamping(0.0f),
    m_material(g_physicsManager->defaultMaterial()),
    m_updatingTransform(false),
    m_btRigidBody(nullptr),
    m_btCompoundShape(nullptr),
    m_motionState(new MotionState(this))
{}

/** Destroy the rigid body. */
RigidBody::~RigidBody() {
    check(!m_btRigidBody);
    check(!m_btCompoundShape);
}

/**
 * Set the mass of the body.
 *
 * Sets the mass of the body. If this is set to 0, then the body will become a
 * static body, i.e. it will not be affected by gravity, but it will collide
 * with other bodies.
 *
 * @param mass          New mass.
 */
void RigidBody::setMass(float mass) {
    m_mass = mass;

    if (m_btRigidBody) {
        btCollisionShape *shape = getShape();
        btVector3 inertia(0.0f, 0.0f, 0.0f);
        shape->calculateLocalInertia(m_mass, inertia);
        m_btRigidBody->setMassProps(m_mass, inertia);
    }
}

/** Set the linear damping factor.
 * @param damping       New linear damping factor. */
void RigidBody::setLinearDamping(float damping) {
    m_linearDamping = damping;

    if (m_btRigidBody)
        m_btRigidBody->setDamping(m_linearDamping, m_angularDamping);
}

/** Set the angular damping factor.
 * @param damping       New angular damping factor. */
void RigidBody::setAngularDamping(float damping) {
    m_angularDamping = damping;

    if (m_btRigidBody)
        m_btRigidBody->setDamping(m_linearDamping, m_angularDamping);
}

/** Set the physics material for the body.
 * @param material      New physics material. */
void RigidBody::setMaterial(PhysicsMaterial *material) {
    check(material);

    m_material = material;

    if (m_btRigidBody) {
        m_btRigidBody->setRestitution(m_material->restitution());
        m_btRigidBody->setFriction(m_material->friction());
    }
}

/** @return             Current velocity of the body. */
glm::vec3 RigidBody::velocity() {
    check(m_btRigidBody);
    return BulletUtil::fromBullet(m_btRigidBody->getLinearVelocity());
}

/** @return             Current angular velocity of the body. */
glm::vec3 RigidBody::angularVelocity() {
    check(m_btRigidBody);
    return BulletUtil::fromBullet(m_btRigidBody->getAngularVelocity());
}

/**
 * Set the linear velocity of the body.
 *
 * Sets the linear velocity of the body. Do not do this regularly as it will
 * result in unrealistic behaviour.
 *
 * @param velocity      New velocity for the body.
 */
void RigidBody::setVelocity(const glm::vec3 &velocity) {
    check(m_btRigidBody);
    m_btRigidBody->setLinearVelocity(BulletUtil::toBullet(velocity));
}

/**
 * Set the angular velocity of the body.
 *
 * Sets the angular velocity of the body. Do not do this regularly as it will
 * result in unrealistic behaviour.
 *
 * @param velocity      New angular velocity for the body.
 */
void RigidBody::setAngularVelocity(const glm::vec3 &velocity) {
    check(m_btRigidBody);
    m_btRigidBody->setAngularVelocity(BulletUtil::toBullet(velocity));
}

/**
 * Get the current shape for the body.
 *
 * Gets the current shape for the body. If the body only has one shape attached
 * to the same entity, then that will be returned, otherwise the compound shape
 * will be returned.
 *
 * @return              Current shape for the body.
 */
btCollisionShape *RigidBody::getShape() const {
    return (m_btCompoundShape)
        ? m_btCompoundShape
        : m_btRigidBody->getCollisionShape();
}

/** Calculate the local transformation for a CollisionShape.
 * @param rigidBody     Rigid body being added to.
 * @param shape         Shape being added. */
static inline btTransform calculateLocalTransform(
    const RigidBody *rigidBody,
    const CollisionShape *shape)
{
    glm::vec3 position = shape->worldPosition() - rigidBody->worldPosition();
    glm::quat orientation = Math::quatDifference(
        rigidBody->worldOrientation(),
        shape->worldOrientation());
    return btTransform(BulletUtil::toBullet(orientation), BulletUtil::toBullet(position));
}

/** Add a shape to the body (callback from CollisionShape).
 * @param shape         Shape to add to the body. */
void RigidBody::addShape(CollisionShape *shape) {
    check(activeInWorld());

    shape->m_rigidBody = this;

    /* If we don't have a compound shape yet and this is not the first shape or
     * the shape being added is attached to a child entity (and therefore needs
     * a transformation relative to the body), we must create a compound shape. */
    if (!m_btCompoundShape && (m_btRigidBody || shape->entity() != entity())) {
        m_btCompoundShape = new btCompoundShape();

        if (m_btRigidBody) {
            /* Move existing shape over to the compound. Since it was attached
             * directly it must exist on the same entity as the body and
             * therefore has an identity transformation. */
            CollisionShape *existing = CollisionShape::fromBtShape(m_btRigidBody->getCollisionShape());
            m_btCompoundShape->addChildShape(btTransform::getIdentity(), existing->m_btShape.get());

            /* Now switch the body to the compound shape. */
            m_btRigidBody->setCollisionShape(m_btCompoundShape);
        }
    }

    /* If we have a compound shape, add the shape to it. */
    if (m_btCompoundShape) {
        btTransform localTransform = calculateLocalTransform(this, shape);
        m_btCompoundShape->addChildShape(localTransform, shape->m_btShape.get());
    }

    /* Create the body if we don't have one yet. */
    if (!m_btRigidBody) {
        btCollisionShape *bodyShape = (m_btCompoundShape)
            ? m_btCompoundShape
            : shape->m_btShape.get();

        btVector3 inertia(0.0f, 0.0f, 0.0f);
        bodyShape->calculateLocalInertia(m_mass, inertia);

        btRigidBody::btRigidBodyConstructionInfo constructionInfo(
            m_mass,
            m_motionState.get(),
            bodyShape,
            inertia);
        constructionInfo.m_linearDamping = m_linearDamping;
        constructionInfo.m_angularDamping = m_angularDamping;
        constructionInfo.m_friction = m_material->friction();
        constructionInfo.m_restitution = m_material->restitution();

        m_btRigidBody = new btRigidBody(constructionInfo);

        PhysicsWorld *world = entity()->world()->physics();
        world->m_btWorld->addRigidBody(m_btRigidBody);
    }
}

/** Remove a shape from the body (callback from CollisionShape).
 * @param shape         Shape to remove from the body. */
void RigidBody::removeShape(CollisionShape *shape) {
    check(m_btRigidBody);

    bool destroyBody;

    if (m_btCompoundShape) {
        /* Remove the shape from the compound. */
        m_btCompoundShape->removeChildShape(shape->m_btShape.get());
        destroyBody = m_btCompoundShape->getNumChildShapes() == 0;
    } else {
        /* Only shape attached to the body. */
        check(m_btRigidBody->getCollisionShape() == shape->m_btShape.get());
        destroyBody = true;
    }

    if (destroyBody) {
        PhysicsWorld *world = entity()->world()->physics();
        world->m_btWorld->removeRigidBody(m_btRigidBody);

        delete m_btRigidBody;
        m_btRigidBody = nullptr;

        if (m_btCompoundShape) {
            delete m_btCompoundShape;
            m_btCompoundShape = nullptr;
        }
    }

    shape->m_rigidBody = nullptr;
}

/** Update a shape's Bullet shape object (callback from CollisionShape).
 * @param shape         Shape to change Bullet shape for.
 * @param btShape       New Bullet shape. */
void RigidBody::updateShape(CollisionShape *shape, btCollisionShape *btShape) {
    btCollisionShape *old = shape->m_btShape.get();

    if (m_btCompoundShape) {
        btTransform localTransform = calculateLocalTransform(this, shape);
        m_btCompoundShape->addChildShape(localTransform, btShape);
        m_btCompoundShape->removeChildShape(old);
    } else {
        check(m_btRigidBody->getCollisionShape() == old);
        m_btRigidBody->setCollisionShape(btShape);
    }
}

/** Update a shape's local transformation (callback from CollisionShape).
 * @param shape         Shape to update transformation for. */
void RigidBody::transformShape(CollisionShape *shape) {
    /* Don't need to do anything if the shape is attached to same entity. */
    if (!m_updatingTransform && shape->entity() != entity()) {
        check(m_btCompoundShape);

        /* Sigh, no pointer based API, do it manually... */
        btCollisionShape *btShape = shape->m_btShape.get();
        for (int i = 0; i < m_btCompoundShape->getNumChildShapes(); i++) {
            if (m_btCompoundShape->getChildShape(i) == btShape) {
                btTransform localTransform = calculateLocalTransform(this, shape);
                m_btCompoundShape->updateChildTransform(i, localTransform);
                break;
            }
        }
    }
}

/** Called when the entity's transformation is changed. */
void RigidBody::transformed() {
    if (m_btRigidBody && !m_updatingTransform) {
        btTransform transform(
            BulletUtil::toBullet(orientation()),
            BulletUtil::toBullet(position()));

        m_btRigidBody->setWorldTransform(transform);
    }
}

/** Called when the component becomes active in the world. */
void RigidBody::activated() {
    #ifdef ORION_BUILD_DEBUG
        /* Ensure no other RigidBody components are above us. */
        for (Entity *parent = entity()->parent(); parent; parent = parent->parent()) {
            RigidBody *other = parent->findComponent<RigidBody>();
            checkMsg(!other, "Activating RigidBody as child of another");
        }
    #endif

    /* Scan down for active CollisionShapes that we should take control of.
     * Note that the body will only truly become active once it also has at
     * least one shape, so creation of the body is deferred until addShape(). */
    std::function<void (Entity *)> addShapes =
        [&] (Entity *entity) {
            CollisionShape *shape = entity->findComponent<CollisionShape>();
            if (shape && shape->activeInWorld())
                addShape(shape);

            entity->visitActiveChildren(addShapes);
        };
    addShapes(entity());
}

/** Called when the component becomes inactive in the world. */
void RigidBody::deactivated() {
    /* Scan down for active CollisionShapes that we should release control of. */
    std::function<void (Entity *)> removeShapes =
        [&] (Entity *entity) {
            CollisionShape *shape = entity->findComponent<CollisionShape>();
            if (shape && shape->activeInWorld())
                removeShape(shape);

            entity->visitActiveChildren(removeShapes);
        };
    removeShapes(entity());

    /* Should be destroyed by the removal of shapes. */
    check(!m_btRigidBody);
}

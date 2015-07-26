/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics system internal definitions.
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic pop

#include "core/core.h"

extern btCollisionConfiguration *g_btCollisionConfiguration;
extern btDispatcher *g_btDispatcher;
extern btBroadphaseInterface *g_btBroadphase;
extern btConstraintSolver *g_btConstraintSolver;

namespace BulletUtil {
    /** Convert a vector to a Bullet vector.
     * @param vector        Vector to convert.
     * @return              Converted Bullet vector. */
    static inline btVector3 toBullet(const glm::vec3 &vector) {
        return btVector3(vector.x, vector.y, vector.z);
    }

    /** Convert a vector from a Bullet vector.
     * @param vector        Bullet vector to convert.
     * @return              Converted vector. */
    static inline glm::vec3 fromBullet(const btVector3 &vector) {
        return glm::vec3(vector.x(), vector.y(), vector.z());
    }

    /** Convert a quaternion to a Bullet quaternion.
     * @param quat          Quaternion to convert.
     * @return              Converted Bullet quaternion. */
    static inline btQuaternion toBullet(const glm::quat &quat) {
        return btQuaternion(quat.x, quat.y, quat.z, quat.w);
    }

    /** Convert a quaternion from a Bullet quaternion.
     * @param quat          Bullet quaternion to convert.
     * @return              Converted quaternion. */
    static inline glm::quat fromBullet(const btQuaternion &quat) {
        return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
    }
};

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 */

#ifndef ORION_WORLD_CAMERA_H
#define ORION_WORLD_CAMERA_H

#include "math/rect.h"

#include "world/component.h"

class SceneView;

/** A view into the world from which the scene will be rendered. */
class Camera : public Component {
public:
	ORION_COMPONENT(Component::kCameraType);
public:
	explicit Camera(Entity *entity);

	/**
	 * Projection manipulation.
	 */

	void perspective(float fovx, float znear = 0.1f, float zfar = 1000.0f);
	void set_fov(float fovx);
	void set_znear(float znear);
	void set_zfar(float zfar);

	/** @return		Horizontal field of view. */
	float fovx() const { return m_fovx; }
	/** @return		Near clipping plane. */
	float znear() const { return m_znear; }
	/** @return		Far clipping plane. */
	float zfar() const { return m_zfar; }

	/**
	 * Viewport manipulation.
	 */

	void set_viewport(const Rect &viewport);

	/** @return		Normalized viewport rectangle. */
	const Rect &viewport() const { return m_viewport; }

	// XXX: Temporary.
	SceneView *scene_view() const { return m_scene_view; }
protected:
	~Camera();

	void transformed();
	void activated();
	void deactivated();
private:
	void update_projection();
	void update_viewport();
private:
	/** Scene view implementing this camera. */
	SceneView *m_scene_view;

	/** Projection settings. */
	float m_fovx;			/**< Horizontal field of view. */
	float m_znear;			/**< Near clipping plane. */
	float m_zfar;			/**< Far clipping plane. */

	/** Normalized viewport rectangle. */
	Rect m_viewport;
};

#endif /* ORION_WORLD_CAMERA_H */

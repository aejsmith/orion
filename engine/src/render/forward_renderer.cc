/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#include "engine/material.h"

#include "render/defs.h"
#include "render/scene.h"
#include "render/scene_light.h"
#include "render/scene_view.h"

#include "forward_renderer.h"

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
ForwardRenderer::ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params) :
	SceneRenderer(scene, target, params)
{}

/** Render the scene.
 * @param view		View to render from. */
void ForwardRenderer::render(SceneView *view) {
	/* Get a list of all visible entities. */
	std::list<SceneEntity *> entities;
	m_scene->visitVisibleEntities(view, [&entities](SceneEntity *e) { entities.push_back(e); });

	/* Get a list of all lights affecting the view. */
	std::list<SceneLight *> lights;
	m_scene->visitVisibleLights(view, [&lights](SceneLight *l) { lights.push_back(l); });

	/* Set view uniforms once per frame. */
	g_gpu->bindUniformBuffer(UniformSlots::kViewUniforms, view->uniforms());

	/* For the first light we don't need blending, and want depth writes on.
	 * FIXME: These should be defaults for the GPU interface here and when
	 * this is called this should be expected to be the current state. */
	g_gpu->setBlendMode();
	g_gpu->setDepthMode();

	/* Render all visible entities for each light to accumulate the lighting
	 * contributions. */
	for(SceneLight *light : lights) {
		g_gpu->bindUniformBuffer(UniformSlots::kLightUniforms, light->uniforms());

		for(SceneEntity *entity : entities) {
			g_gpu->bindUniformBuffer(UniformSlots::kEntityUniforms, entity->uniforms());

			Material *material = entity->material();
			material->shader()->setDrawState(material);
			material->shader()->pass(Pass::kForwardPass, 0)->setDrawState();

			entity->draw();
		}

		/* After the first iteration, we want to blend the remaining
		 * lights, and we can turn depth writes off. */
		g_gpu->setBlendMode(BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne);
		g_gpu->setDepthMode(ComparisonFunc::kEqual, true);
	}
}

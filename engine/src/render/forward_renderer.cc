/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#include "gpu/pipeline.h"

#include "render/defs.h"
#include "render/scene.h"
#include "render/scene_light.h"
#include "render/scene_view.h"

#include "forward_renderer.h"

// Temporary until we have proper shader/material management.
static GlobalGPUResource<GPUPipeline> g_lightingPipeline;

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
ForwardRenderer::ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params) :
	SceneRenderer(scene, target, params)
{
	if(unlikely(!g_lightingPipeline)) {
		GPUShaderPtr vertexShader = g_gpu->loadShader(
			"engine/shaders/forward_light_vtx.glsl",
			GPUShader::kVertexShader);
		vertexShader->bindUniforms("EntityUniforms", UniformSlots::kEntityUniforms);
		vertexShader->bindUniforms("ViewUniforms", UniformSlots::kViewUniforms);

		GPUShaderPtr fragShader = g_gpu->loadShader(
			"engine/shaders/forward_light_frag.glsl",
			GPUShader::kFragmentShader);
		fragShader->bindUniforms("ViewUniforms", UniformSlots::kViewUniforms);
		fragShader->bindUniforms("LightUniforms", UniformSlots::kLightUniforms);
		fragShader->bindTexture("diffuseTexture", 0);

		g_lightingPipeline() = g_gpu->createPipeline();
		g_lightingPipeline->addShader(vertexShader);
		g_lightingPipeline->addShader(fragShader);
		g_lightingPipeline->finalize();
	}
}

/** Render the scene.
 * @param view		View to render from. */
void ForwardRenderer::render(SceneView *view) {
	/* Get a list of all visible entities. */
	std::list<SceneEntity *> entities;
	m_scene->visitVisibleEntities(view, [&entities](SceneEntity *e) { entities.push_back(e); });

	/* Get a list of all lights affecting the view. */
	std::list<SceneLight *> lights;
	m_scene->visitVisibleLights(view, [&lights](SceneLight *l) { lights.push_back(l); });

	/* Set view and scene uniforms once per frame. */
	g_gpu->bindUniformBuffer(UniformSlots::kViewUniforms, view->uniforms());

	// Temporary.
	g_gpu->bindPipeline(g_lightingPipeline);

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
			entity->render();
		}

		/* After the first iteration, we want to blend the remaining
		 * lights, and we can turn depth writes off. */
		g_gpu->setBlendMode(BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne);
		g_gpu->setDepthMode(ComparisonFunc::kEqual, true);
	}
}

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#include "gpu/pipeline.h"

#include "render/scene.h"
#include "render/scene_light.h"
#include "render/scene_view.h"

#include "shader/defs.h"

#include "forward_renderer.h"

// Temporary until we have proper shader/material management.
static GPUPipelinePtr lighting_pipeline;

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
ForwardRenderer::ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params) :
	SceneRenderer(scene, target, params)
{
	if(unlikely(!lighting_pipeline)) {
		GPUProgramPtr vertex_program = g_engine->gpu()->load_program(
			"engine/assets/shaders/forward_light_vtx.glsl",
			GPUProgram::kVertexProgram);
		vertex_program->bind_uniforms("EntityUniforms", ShaderUniforms::kEntityUniforms);
		vertex_program->bind_uniforms("ViewUniforms", ShaderUniforms::kViewUniforms);

		GPUProgramPtr frag_program = g_engine->gpu()->load_program(
			"engine/assets/shaders/forward_light_frag.glsl",
			GPUProgram::kFragmentProgram);
		frag_program->bind_uniforms("ViewUniforms", ShaderUniforms::kViewUniforms);
		frag_program->bind_uniforms("LightUniforms", ShaderUniforms::kLightUniforms);
		frag_program->bind_texture("diffuse_texture", 0);

		lighting_pipeline = g_engine->gpu()->create_pipeline();
		lighting_pipeline->set_program(GPUProgram::kVertexProgram, vertex_program);
		lighting_pipeline->set_program(GPUProgram::kFragmentProgram, frag_program);
		lighting_pipeline->finalize();
	}
}

/** Render the scene.
 * @param view		View to render from. */
void ForwardRenderer::render(SceneView *view) {
	/* Get a list of all visible entities. */
	std::list<SceneEntity *> entities;
	m_scene->visit_visible_entities(view, [&entities](SceneEntity *e) { entities.push_back(e); });

	/* Get a list of all lights affecting the view. */
	std::list<SceneLight *> lights;
	m_scene->visit_visible_lights(view, [&lights](SceneLight *l) { lights.push_back(l); });

	/* Set view and scene uniforms once per frame. */
	g_engine->gpu()->bind_uniform_buffer(ShaderUniforms::kViewUniforms, view->uniforms());

	// Temporary.
	g_engine->gpu()->bind_pipeline(lighting_pipeline);

	/* For the first light we don't need blending, and want depth writes on.
	 * FIXME: These should be defaults for the GPU interface here and when
	 * this is called this should be expected to be the current state. */
	g_engine->gpu()->set_blend_mode();
	g_engine->gpu()->set_depth_mode();

	/* Render all visible entities for each light to accumulate the lighting
	 * contributions. */
	for(SceneLight *light : lights) {
		g_engine->gpu()->bind_uniform_buffer(ShaderUniforms::kLightUniforms, light->uniforms());

		for(SceneEntity *entity : entities) {
			g_engine->gpu()->bind_uniform_buffer(ShaderUniforms::kEntityUniforms, entity->uniforms());
			entity->render();
		}

		/* After the first iteration, we want to blend the remaining
		 * lights, and we can turn depth writes off. */
		g_engine->gpu()->set_blend_mode(BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne);
		g_engine->gpu()->set_depth_mode(ComparisonFunc::kEqual, true);
	}
}

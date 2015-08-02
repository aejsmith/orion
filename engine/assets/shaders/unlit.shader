{
    "parameters": {
        "diffuseTexture": {"type": "Texture"}
    },
    "passes": [
        {
            "type": "Basic",
            "vertex": {"source": "engine/shaders/unlit_vtx.glsl"},
            "fragment": {
                "source": "engine/shaders/unlit_frag.glsl",
                "keywords": ["TEXTURED"]
            }
        }, {
            "type": "ShadowCaster",
            "vertex": {"source": "engine/shaders/shadow_vtx.glsl"},
            "fragment": {"source": "engine/shaders/shadow_frag.glsl"}
        }
    ]
}
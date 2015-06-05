{
    "parameters": {
        "shininess": {"type": "Float"},
        "specularColour": {"type": "Vec3"},
        "diffuseTexture": {"type": "Texture"}
    },
    "passes": [
        {
            "type": "Deferred",
            "vertex": {"source": "engine/shaders/lit_vtx.glsl"},
            "fragment": {
                "source": "engine/shaders/lit_deferred_frag.glsl",
                "keywords": ["SPECULAR", "TEXTURED"]
            }
        }, {
            "type": "Forward",
            "vertex": {"source": "engine/shaders/lit_vtx.glsl"},
            "fragment": {
                "source": "engine/shaders/lit_frag.glsl",
                "keywords": ["SPECULAR", "TEXTURED"]
            }
        }, {
            "type": "ShadowCaster",
            "vertex": {"source": "engine/shaders/shadow_vtx.glsl"},
            "fragment": {"source": "engine/shaders/shadow_frag.glsl"}
        }
    ]
}

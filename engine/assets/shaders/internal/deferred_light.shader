{
    "passes": [
        {
            "type": "Forward",
            "vertex": {"source": "engine/shaders/deferred_light_vtx.glsl"},
            "fragment": {
                "source": "engine/shaders/deferred_light_frag.glsl",
                "keywords": ["SPECULAR"]
            }
        }
    ]
}

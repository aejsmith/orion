{
    "parameters": {
        "font": {"type": "Texture"},
        "colour": {"type": "Vec3"}
    },
    "passes": [
        {
            "type": "Basic",
            "vertex": {"source": "engine/shaders/debug_text_vtx.glsl"},
            "fragment": {"source": "engine/shaders/debug_text_frag.glsl"}
        }
    ]
}

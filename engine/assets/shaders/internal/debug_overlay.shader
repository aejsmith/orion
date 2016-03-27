{
    "parameters": {
        "projectionMatrix": {"type": "Mat4"}
    },
    "passes": [
        {
            "type": "Basic",
            "vertex": {"source": "engine/shaders/debug_overlay_vtx.glsl"},
            "fragment": {"source": "engine/shaders/debug_overlay_frag.glsl"}
        }
    ]
}

{
    "parameters": {
        "skybox": {"type": "Texture"}
    },
    "passes": [
        {
            "type": "Basic",
            "vertex": {"source": "engine/shaders/skybox_vtx.glsl"},
            "fragment": {"source": "engine/shaders/skybox_frag.glsl"}
        }
    ]
}

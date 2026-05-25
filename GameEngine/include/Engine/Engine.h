#pragma once
#include "EngineAPI.h"
#include "Scene.h"

extern "C"
{
    class ENGINE_API Engine {
    public:
        std::unique_ptr<Scene> scene;

        void Init();
    };

    
    ENGINE_API Engine* GetEngine();
    ENGINE_API void DestroyEngine(Engine* engine);
}
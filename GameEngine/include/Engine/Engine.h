#pragma once
#include "EngineAPI.h"

extern "C"
{
    class ENGINE_API Engine {
    public:
        void Init();
        void Render();
    };

    
    ENGINE_API Engine* GetEngine();
    ENGINE_API void DestroyEngine(Engine* engine);
}
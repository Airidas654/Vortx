#pragma once
#include "EngineAPI.h"
#include "Scene.h"


class ENGINE_API Engine {
public:
    void Init();
};

extern "C"
{
    
    ENGINE_API Engine* GetEngine();
    ENGINE_API void DestroyEngine(Engine* engine);
}
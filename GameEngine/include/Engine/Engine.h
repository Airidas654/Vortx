#pragma once
#include "EngineAPI.h"

class ENGINE_API Engine {
public:
    void Init();
    void Render();
};

// Free functions too
ENGINE_API void EngineInit();
ENGINE_API void EngineShutdown();
#pragma once
#include "EngineAPI.h"
#include "SimpleAPI/GameObject.h"

namespace Vortx
{
    class ENGINE_API ScriptInstance
    {
    public:
        virtual ~ScriptInstance() = default;

        virtual void Start() {}
        virtual void Update() {}
        virtual void Destroy() {}

        bool _started = false;
        GameObject gameObject;
    };
}
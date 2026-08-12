#pragma once
#include "Scripting/ScriptInstance.h"
#include "Scripting/ScriptRegistry.h"
#include <typeinfo>

#include <iostream>

namespace Vortx
{
    template<typename T>
    class ScriptBase : public ScriptInstance
    {
    public:
        
        static bool Register()
        {
            ScriptRegistry::ScriptInfo info;
            info.name = typeid(T).name();
            info.create = []() -> ScriptInstance* { return new T(); };
            info.destroy = [](ScriptInstance* s) { delete static_cast<T*>(s); };

            ScriptRegistry::Instance().Register(std::move(info));
            return true;
        }

        void Start() override {}
        void Update() override {}
        void FixedUpdate() override {}
        void Destroy() override {}
        void OnDrawGizmos() override {}
        void OnDrawGizmosSelected() override {}
    };
}

#define REGISTER_SCRIPT(T) inline bool T##_registered = Vortx::ScriptBase<T>::Register();
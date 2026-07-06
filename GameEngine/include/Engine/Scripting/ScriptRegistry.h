#pragma once
#include "EngineAPI.h"
#include "Scripting/ScriptInstance.h"
#include <string>
#include <unordered_map>

namespace Vortx
{

    class ENGINE_API ScriptRegistry
    {
    public:
        static ScriptRegistry& Instance();

        struct ScriptInfo
        {
            const char* name;
            ScriptInstance* (*create)();
            void (*destroy)(ScriptInstance*);
        };

        void Register(ScriptInfo info);
        void ResetGame();

        ScriptInstance* Create(const std::string& name);

        const std::unordered_map<std::string, ScriptInfo>& GetAll() const;

    private:
        ScriptRegistry();
        ~ScriptRegistry();
        struct Impl;
        Impl* impl;
    };
}
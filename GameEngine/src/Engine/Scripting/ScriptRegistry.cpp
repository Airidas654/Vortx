#include "Scripting/ScriptRegistry.h"
#include <iostream>

namespace Vortx
{

    struct ScriptRegistry::Impl
    {
        std::unordered_map<std::string, ScriptInfo> scripts;
    };

    ScriptRegistry::ScriptRegistry() : impl(new Impl()) {}
    ScriptRegistry::~ScriptRegistry() { delete impl; }

    ScriptRegistry& ScriptRegistry::Instance()
    {
        static ScriptRegistry* instance = nullptr;
        if (!instance)
        {
            instance = new ScriptRegistry();
            std::atexit([]() { delete instance; instance = nullptr; });
        }
        return *instance;
    }

    void ScriptRegistry::Register(ScriptInfo info)
    {
        std::string name = info.name;
        impl->scripts[info.name] = std::move(info);
    }

    void ScriptRegistry::ResetGame()
    {
        impl->scripts.clear();
    }

    ScriptInstance* ScriptRegistry::Create(const std::string& name)
    {
        auto it = impl->scripts.find(name);
        if (it == impl->scripts.end())
        {
            std::cerr << "[ScriptRegistry] Script not found: " << name << "\n";
            return nullptr;
        }
        return it->second.create();
    }

    const std::unordered_map<std::string, ScriptRegistry::ScriptInfo>&
        ScriptRegistry::GetAll() const
    {
        return impl->scripts;
    }

}
#include "Scripting\ScriptSystem.h"

#include "Scripting\ScriptBase.h"

#include "Scene.h"

#include <iostream>

struct ScriptSystem::Impl
{
    std::vector<ScriptEntry> entries;
};

ScriptSystem::ScriptSystem() : impl(new Impl()) {}
ScriptSystem::~ScriptSystem() { delete impl; }

ScriptSystem* ScriptSystem::instance = nullptr;

void ScriptSystem::Register(ScriptEntry entry)
{
    impl->entries.push_back(entry);

    entry.addComponent(Scene::Current()->GetRegistry(), Scene::Current()->GetChildren()[0].GetEntity());
    //Scene::Current()->GetRegistry().
    //Scene::Current()->GetChildren()[0].AddComponent<>();
}

ScriptSystem& ScriptSystem::Instance()
{
    if (!instance)
    {
        instance = new ScriptSystem();

        std::atexit([]()
        {
            delete instance;
            instance = nullptr;
        });
        std::cout << "instance created " << instance << std::endl;
    }


    return *instance;
}

// ScriptSystem.cpp
void ScriptSystem::Update()
{
    auto& registry = Scene::Current()->GetRegistry();

    for (auto& entry : impl->entries)
    {
        auto* storage = registry.Storage(entry.typeId);
        if (!storage) continue;

        for (auto entity : *storage)
        {
            void* raw = storage->value(entity);
            
            if (!raw) continue;
            

            ScriptInstanceBase* base = reinterpret_cast<ScriptInstanceBase*>(raw);

            if (!base->_started)
            {
                entry.start(base);
                base->_started = true;
            }
            else
            {
                entry.update(base);
            }
        }
    }
}

void ScriptSystem::Destroy()
{
    //for (auto& fn : impl->destroyFns) 
       // fn(Scene::Current()->GetRegistry());
}

void ScriptSystem::Reset()
{
    impl->entries.clear();
}
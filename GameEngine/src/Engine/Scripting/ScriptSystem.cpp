#include "Scripting/ScriptSystem.h"
#include "Scripting/ScriptRegistry.h"
#include "SimpleAPI/SceneManager.h"
#include "Scene.h"
#include <iostream>

namespace Vortx
{

    ScriptSystem& ScriptSystem::Instance()
    {
        static ScriptSystem* instance = nullptr;
        if (!instance)
        {
            instance = new ScriptSystem();
            std::atexit([]() { delete instance; instance = nullptr; });
        }
        return *instance;
    }

    void ScriptSystem::Update()
    {
        if (!Vortx::SceneManager::current) return;

        Vortx::SceneManager::current->GetRegistry()
            .View<ScriptComponent>()
            .each([](ScriptComponent& sc)
        {
            for (auto& entry : sc.scripts)
            {
                if (!entry.instance) continue;

                if (!entry.instance->_started)
                {
                    entry.instance->Start();
                    entry.instance->_started = true;
                }
                else
                {
                    entry.instance->Update();
                }
            }
        });
    }

    void ScriptSystem::DestroyAll()
    {
        if (!Vortx::SceneManager::current) return;

        Vortx::SceneManager::current->GetRegistry()
            .View<ScriptComponent>()
            .each([](ScriptComponent& sc)
        {
            for (auto& entry : sc.scripts)
            {
                if (!entry.instance) continue;

                entry.instance->Destroy();

                auto& all = ScriptRegistry::Instance().GetAll();
                auto it = all.find(entry.name);
                if (it != all.end())
                    it->second.destroy(entry.instance);
                else
                    delete entry.instance;

                entry.instance = nullptr;
            }
        });
    }

    void ScriptSystem::CreateOne(Entity& entity, ScriptComponent::Entry &entry)
    {
        GameObject go(entity);

        if (entry.instance) return;

        entry.instance = ScriptRegistry::Instance().Create(entry.name);
        if (!entry.instance) return;

        entry.instance->_started = false;
        entry.instance->gameObject = go;
    }

    void ScriptSystem::CreateAll()
    {
        if (!Vortx::SceneManager::current) return;

        Registry& reg = Vortx::SceneManager::current->GetRegistry();

        reg.View<ScriptComponent>().each([&reg](entt::entity e, ScriptComponent& sc)
        {
            Entity entity(&reg, e);
            GameObject go(entity);

            for (auto& entry : sc.scripts)
            {
                if (entry.instance) continue; 

                entry.instance = ScriptRegistry::Instance().Create(entry.name);
                if (!entry.instance) continue;

                entry.instance->_started = false;
                entry.instance->gameObject = go;
            }
        });
    }

    void ScriptSystem::AddScript(Entity& entity, const std::string& name)
    {
        if (!entity.HasComponent<ScriptComponent>())
            entity.AddComponent<ScriptComponent>();

        auto& sc = entity.GetComponent<ScriptComponent>();

        ScriptComponent::Entry entry;
        entry.name = name;
        entry.instance = nullptr;

        sc.scripts.push_back(std::move(entry));

        CreateOne(entity, sc.scripts[sc.scripts.size() - 1]);
    }

    void ScriptSystem::RemoveScript(Entity& entity, const std::string& name)
    {
        if (!entity.HasComponent<ScriptComponent>()) return;

        auto& sc = entity.GetComponent<ScriptComponent>();

        for (auto it = sc.scripts.begin(); it != sc.scripts.end(); ++it)
        {
            if (it->name != name) continue;

            if (it->instance)
            {
                it->instance->Destroy();
                auto& all = ScriptRegistry::Instance().GetAll();
                auto fit = all.find(name);
                if (fit != all.end())
                    fit->second.destroy(it->instance);
                else
                    delete it->instance;
            }

            sc.scripts.erase(it);

            if (sc.scripts.size() == 0) 
            {
                entity.RemoveComponent<ScriptComponent>();
            }

            return;
        }
    }
}
#pragma once
#include "EngineAPI.h"
#include "ECS/Entity.h"
#include "Components/ComponentScript.h"

#include <string>

namespace Vortx
{

    class ENGINE_API ScriptSystem
    {
    public:
        static ScriptSystem& Instance();

        void Update();
        void FixedUpdate();
        void SceneUpdate(AssetID selected);

        void DestroyAll();

        void CreateOne(Entity& entity, ScriptComponent::Entry& entry);
        void CreateAll();

        void AddScript(Entity& entity, const std::string& name);
        void RemoveScript(Entity& entity, const std::string& name);

    private:
        ScriptSystem() = default;
        ~ScriptSystem() = default;
    };
}
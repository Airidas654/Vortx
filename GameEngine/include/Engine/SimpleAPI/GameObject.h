#pragma once

#include "ECS/Entity.h"
#include "EngineAPI.h"
#include "SimpleAPI/BaseComponent.h"

class ENGINE_API GameObject : public BaseComponent
{
private:
    void _InternalCreate() override;

public:
    GameObject(Entity entity) : BaseComponent(entity) {}

    GameObject(Entity entity, bool create);

    const std::string& NameGet();
    void NameSet(const std::string& name);

    const std::string& TagGet();
    void TagSet(const std::string& tag);
};
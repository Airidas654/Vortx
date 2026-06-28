#pragma once

#include "ECS/Registry.h"
#include "ECS\Entity.h"

#include "EngineAPI.h"
//#include "PropertiesLoader.h"

#include <iostream>

struct ScriptEntry
{
    entt::id_type typeId;
    void (*addComponent)(Registry& reg, Entity e);
    void (*start)(void* script);
    void (*update)(void* script);
    void (*destroy)(void* script);
};

class ENGINE_API ScriptSystem {
public:
    static ScriptSystem& Instance();

    void Register(ScriptEntry entry);

    void Update();
    void Destroy();
    void Reset();

private:
    ScriptSystem();
    ~ScriptSystem();

    struct Impl;     // forward declare — Game.dll never sees the fields
    Impl* impl;      // pImpl: only a pointer, always same size

    static ScriptSystem* instance;
};
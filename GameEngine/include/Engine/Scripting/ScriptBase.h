#pragma once

#include "Scripting/ScriptSystem.h"

#include "Scripting\ScriptInstanceBase.h"

#include <iostream>

class GameObject;

template<typename Derived>
class ScriptBase : ScriptInstanceBase {

    static void DoStart(void* ptr) { static_cast<Derived*>(ptr)->Start(); }
    static void DoUpdate(void* ptr) { static_cast<Derived*>(ptr)->Update(); }
    static void DoDestroy(void* ptr) { static_cast<Derived*>(ptr)->Destroy(); }

    static void AddComponent(Registry &reg, Entity e)
    {
        e.AddComponent<Derived>();
        //reg.AddComponent<Derived>(e);
    }

    inline static bool _ = []()
    {
        ScriptEntry entry;
        entry.typeId = entt::type_hash<Derived>::value();
        entry.start = &DoStart;
        entry.update = &DoUpdate;
        entry.destroy = &DoDestroy;
        entry.addComponent = &AddComponent;
        ScriptSystem::Instance().Register(entry);
        return true;
    }();

public:

    GameObject gameObject;

    void InnerSetGameObject(GameObject& gameObject)
    {
        this->gameObject = gameObject;
    }

    void Start() {}
    void Update() {}
    void Destroy() {}

    ScriptBase()
    {
        std::cout << "constructor" << std::endl;
    }
    
};
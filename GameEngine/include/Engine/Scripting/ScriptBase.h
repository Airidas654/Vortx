#pragma once

#include "Scripting/ScriptSystem.h"

class GameObject;

template<typename Derived>
class ScriptBase {
    inline static bool _ = ScriptSystem::instance().autoRegister<Derived>();

public:

    bool _started;
    GameObject gameObject;

    void InnerSetGameObject(GameObject& gameObject)
    {
        this->gameObject = gameObject;
    }

    void onStart() {}
    void onUpdate() {}
    void onDestroy() {}

    ScriptBase()
    {
        _started = false;
    }
    
};
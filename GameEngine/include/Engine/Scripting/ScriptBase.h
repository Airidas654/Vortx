#pragma once

#include "GameEngineIncludes.h"

template<typename Derived>
class ScriptBase {
    inline static bool _ = ScriptSystem::instance().autoRegister<Derived>();

public:

    GameObject gameObject;

    void InnerSetGameObject(GameObject& gameObject)
    {
        this->gameObject = gameObject;
    }

    void onStart() {}
    void onUpdate() {}
    void onDestroy() {}

    ScriptBase() = default;

protected:
    
};
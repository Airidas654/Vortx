#pragma once

#include "Scripting/ScriptBase.h"

#include <iostream>

class GameTest : public ScriptBase<GameTest> {
public:
    
    GameTest() = default;

    void onStart() {
        std::cout << "started" << std::endl;
    }

    void onUpdate() {
        gameObject.GetComponent<Transform>().Data().position.x += 0.01f;
    }

    void onDestroy() {
        std::cout << "destroyed" << std::endl;
    }

private:
    float speed = 5.0f;
};
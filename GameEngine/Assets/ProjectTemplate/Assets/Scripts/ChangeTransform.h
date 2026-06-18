#pragma once

#include "GameEngineIncludes.h"

#include <iostream>

using namespace Vortx;

class GameTest : public ScriptBase<GameTest> {
public:
    
    void Start() {
        std::cout << "started" << std::endl;
    }

    void Update() {
        gameObject.GetComponent<Transform>().Data().position.x += 1.0f * Time::deltaTime;
    }

    void Destroy() {
        std::cout << "destroyed" << std::endl;
    }

private:
    float speed = 5.0f;
};
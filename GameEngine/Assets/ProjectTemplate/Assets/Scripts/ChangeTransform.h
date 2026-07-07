#pragma once

#include "GameEngineIncludes.h"
//#include ""

#include <iostream>

using namespace Vortx;

class ChangeTransform : public ScriptBase<ChangeTransform> {
public:
    
    float speed = 5.0f;

    void Start()
    {
        std::cout << "MyScript Start on: " << gameObject.NameGet() << std::endl;
    }

    void Update()
    {
        gameObject.GetComponent<Transform>().Position() += glm::vec2(0, speed * Time::deltaTime);
        // gameObject is valid and points to the owning entity
    }

    void Destroy()
    {
        std::cout << "MyScript Destroy\n";
    }

private:
    //float speed = 5.0f;
};

REGISTER_SCRIPT(ChangeTransform)
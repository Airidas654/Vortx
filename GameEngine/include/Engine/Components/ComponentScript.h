#pragma once
#include "EngineAPI.h"
#include "Scripting/ScriptInstance.h"
#include <vector>
#include <string>

namespace Vortx
{

    struct ENGINE_API ScriptComponent
    {
        struct Entry
        {
            std::string name;
            ScriptInstance* instance = nullptr;
        };

        std::vector<Entry> scripts;
    };

}
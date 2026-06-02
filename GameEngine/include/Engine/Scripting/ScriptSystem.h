#pragma once

#include "ECS/Registry.h"

#include "EngineAPI.h"
//#include "PropertiesLoader.h"

class ENGINE_API ScriptSystem {
public:
    static ScriptSystem& instance() {
        static ScriptSystem sys;
        return sys;
    }

    template<typename T>
    bool autoRegister() {
        /*PropertiesLoader::RegisterComponent<T>(
            typeid(T).name(),
            [](Entity entity) {
            ImGui::Text("Script active");
        }
        );*/
        updateFns.push_back([](Registry& reg) {
            reg.View<T>().each([&](auto entity, T& script) {
                if (!script._started) {
                    script.Start();
                    script._started = true;
                }
                else
                {
                    script.Update();
                }
            });
        });
        destroyFns.push_back([](Registry& reg) {
            reg.View<T>().each([&](auto entity, T& script) {
                script.Destroy();
                });
            });
        return true;
    }

    void Update();
    void Destroy();

private:
    std::vector<std::function<void(Registry&)>> updateFns;
    std::vector<std::function<void(Registry&)>> destroyFns;
};
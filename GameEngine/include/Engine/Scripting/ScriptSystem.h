#pragma once

#include "ECS/Registry.h"
#include "Scene.h"

class ScriptSystem {
public:
    static ScriptSystem& instance() {
        static ScriptSystem sys;
        return sys;
    }

    template<typename T>
    bool autoRegister() {
        updateFns.push_back([](Registry& reg) {
            reg.View<T>().each([&](auto entity, T& script) {
                script.onUpdate();
                });
            });
        startFns.push_back([](Registry& reg) {
            reg.View<T>().each([&](auto entity, T& script) {
                script.onStart();
                });
            });
        destroyFns.push_back([](Registry& reg) {
            reg.View<T>().each([&](auto entity, T& script) {
                script.onDestroy();
                });
            });
        return true;
    }

    void onStart() { for (auto& fn : startFns) fn(Scene::current->GetRegistry()); }
    void onUpdate() { for (auto& fn : updateFns) fn(Scene::current->GetRegistry()); }
    void onDestroy() { for (auto& fn : destroyFns) fn(Scene::current->GetRegistry()); }

private:
    std::vector<std::function<void(Registry&)>> startFns;
    std::vector<std::function<void(Registry&)>> updateFns;
    std::vector<std::function<void(Registry&)>> destroyFns;
};
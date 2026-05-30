#include "Scripting\ScriptSystem.h"

#include "Scene.h"

void ScriptSystem::Update() 
{ 
	for (auto& fn : updateFns) fn(Scene::current->GetRegistry()); 
}

void ScriptSystem::Destroy() 
{ 
	for (auto& fn : destroyFns) fn(Scene::current->GetRegistry()); 
}
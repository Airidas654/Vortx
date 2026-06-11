#include "Engine.h"

#include "SimpleAPI/SceneManager.h"

#include <iostream>

Engine* GetEngine()
{
	return new Engine();
}

void DestroyEngine(Engine* engine)
{
	delete engine;
}

void Engine::Init()
{
	std::cout << "engine" << std::endl;

	Vortx::SceneManager::current = std::unique_ptr<Scene>(new Scene());
}
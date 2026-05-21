#include "Engine.h"

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
}
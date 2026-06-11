#pragma once

#include "DynamicLibrary.h"
#include "Engine.h"

class Editor
{
public:
	static void Start(Engine& engine);
	static void Compile();

private:
	static bool isBuilding;
	static bool needsReload;
	static std::string cmakePath;

	static std::string FindCMake();
};
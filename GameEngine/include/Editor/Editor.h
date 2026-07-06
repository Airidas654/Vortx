#pragma once

#include "Core/DynamicLibrary.h"
#include "Engine.h"

using namespace Vortx;

class Editor
{
public:
	static void Start(Engine& engine);
	static void Compile();

private:
	static bool isBuilding;
	static bool needsReload;
	static std::string cmakePath;
	static DynamicLibrary gameLib;

	static void LoadSettings();

	static void GenerateSettings();
};
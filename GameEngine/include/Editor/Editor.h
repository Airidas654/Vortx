#pragma once

#include "DynamicLibrary.h"
#include "Engine.h"

namespace Vortx
{

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

}
#pragma once

#include "EngineAPI.h"
#include "Scene.h"

#include <string>
#include <memory>

namespace Vortx
{
	class ENGINE_API SceneManager
	{
	public:
		static std::unique_ptr<Scene> current;

		static void LoadScene(const uint32_t index);
		static void LoadScene(const std::string &name);

		static uint32_t GetSceneIndex(const std::string& name);

	private:
		

	};
}
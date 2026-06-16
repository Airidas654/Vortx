#include "PlayModeManager.h"

#include "Scene.h"
#include "PropertiesLoader.h"

namespace Vortx
{

	PlayState PlayModeManager::state = PlayState::Editing;
	std::string PlayModeManager::snapshot = "";
	bool PlayModeManager::stepRequested = false;

	void PlayModeManager::EnterPlay() 
	{
		snapshot = Scene::Current()->Serialize();
		state = PlayState::Playing;
	}

	void PlayModeManager::ExitPlay() 
	{
		//ScriptSystem::instance().Destroy();
		Scene::Current()->Deserialize(snapshot);
		//PropertiesLoader::Reset();
		state = PlayState::Editing;
	}

	void PlayModeManager::Pause() 
	{ 
		if (state == PlayState::Playing) state = PlayState::Paused;
	}

	void PlayModeManager::Resume() 
	{ 
		if (state == PlayState::Paused)  state = PlayState::Playing; 
	}

	void PlayModeManager::Step() 
	{
		if (state == PlayState::Paused)
			stepRequested = true;
	}
}
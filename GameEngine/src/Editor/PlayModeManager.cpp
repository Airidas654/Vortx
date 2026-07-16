#include "PlayModeManager.h"
#include "Editor.h"

#include "Scene.h"
#include "PropertiesLoader.h"
#include "Scripting\ScriptSystem.h"

PlayState PlayModeManager::state = PlayState::Editing;
std::vector<uint8_t> PlayModeManager::snapshot;
bool PlayModeManager::stepRequested = false;

void PlayModeManager::EnterPlay()
{
	if (Editor::autoCompile)
	{
		Editor::Compile();

		while (Editor::isBuilding) {}
	}

	snapshot = Scene::Current()->Serialize();
	ScriptSystem::Instance().CreateAll();  
	state = PlayState::Playing;
}

void PlayModeManager::ExitPlay()
{
	ScriptSystem::Instance().DestroyAll();
	Scene::Current()->Deserialize(snapshot);
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
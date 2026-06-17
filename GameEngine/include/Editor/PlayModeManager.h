#pragma once

#include <string>
#include <vector>

namespace Vortx
{

    enum class PlayState { Editing, Playing, Paused };

    class PlayModeManager {
    public:
        static PlayState state;
        static std::vector<uint8_t> snapshot;
        static bool stepRequested;

        static void EnterPlay();

        static void ExitPlay();

        static void Pause();
        static void Resume();

        static void Step();
    };

}
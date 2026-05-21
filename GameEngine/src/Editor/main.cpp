#include "DynamicLibrary.h"
#include "Editor.h"

#include <iostream>

using GetEngineT = Engine * (*)();
using DestroyEngineT = void(*)(Engine*);

int main()
{
#ifdef _WIN32
    const char* libName = "GameEngineLib.dll";
#elif __APPLE__
    const char* libName = "./GameEngineLib.dylib";
#else
    const char* libName = "./GameEngineLib.so";
#endif

    DynamicLibrary lib;

    if (!lib.Load(libName))
    {
        std::cerr << "Failed to load library" << std::endl;
        return 1;
    }

    auto getEngine = lib.GetSymbol<GetEngineT>("GetEngine");
    auto destroyEngine = lib.GetSymbol<DestroyEngineT>("DestroyEngine");

    if (!getEngine || !destroyEngine)
    {
        std::cerr << "Failed to load symbol\n";
        return 1;
    }

    Engine* engine = getEngine();

    Editor::Start(engine);

    destroyEngine(engine);

    return 0;
}
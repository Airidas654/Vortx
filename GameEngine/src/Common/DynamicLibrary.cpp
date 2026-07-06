#include "Core/DynamicLibrary.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <iostream>

namespace Vortx
{

    DynamicLibrary::~DynamicLibrary()
    {
        Unload();
    }

    bool DynamicLibrary::Load(const std::string& path)
    {
        Unload();

#ifdef _WIN32
        handle = (void*)LoadLibraryA(path.c_str());
        if (!handle)
        {
            DWORD err = GetLastError();
            std::cerr << "[DynamicLibrary] Failed to load '" << path
                << "' — WinError: " << err << "\n";
            return false;
        }
#else
        handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle)
        {
            std::cerr << "[DynamicLibrary] Failed to load '" << path
                << "' — " << dlerror() << "\n";
            return false;
        }
#endif

        std::cout << "[DynamicLibrary] Loaded '" << path << "'\n";
        return true;
    }

    void DynamicLibrary::Unload()
    {
        if (!handle)
            return;

#ifdef _WIN32
        FreeLibrary((HMODULE)handle);
#else
        dlclose(handle);
#endif

        handle = nullptr;
    }

    void* DynamicLibrary::GetSymbolInternal(const std::string& name)
    {
        if (!handle)
            return nullptr;

#ifdef _WIN32
        return (void*)GetProcAddress((HMODULE)handle, name.c_str());
#else
        return dlsym(handle, name.c_str());
#endif
    }

    bool DynamicLibrary::IsLoaded() const
    {
        return handle != nullptr;
    }
}
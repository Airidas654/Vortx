#pragma once

#include <string>

class DynamicLibrary
{
public:
    DynamicLibrary() = default;
    ~DynamicLibrary();

    bool Load(const std::string& path);
    void Unload();

    template<typename T>
    T GetSymbol(const std::string& name)
    {
        return reinterpret_cast<T>(GetSymbolInternal(name));
    }

    bool IsLoaded() const;

private:
    void* handle = nullptr;

    void* GetSymbolInternal(const std::string& name);
};
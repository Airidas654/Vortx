#pragma once

#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "Scene.h"
#include "SerializerLoader.h"
#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/SceneManager.h"
#include "SimpleAPI/Transform.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#define EXPECT_VEC2_NEAR(a, b, eps)                    \
    do {                                               \
        auto _va = (a);                                \
        auto _vb = (b);                                \
        EXPECT_NEAR(_va.x, _vb.x, eps);                \
        EXPECT_NEAR(_va.y, _vb.y, eps);                \
    } while (0)

namespace TestUtils
{
    namespace fs = std::filesystem;

    // Unique temp directory, removed on destruction.
    class TempDir
    {
    public:
        TempDir()
        {
            static std::atomic<int> counter{ 0 };
            auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            path_ = fs::temp_directory_path() /
                ("vortx_test_" + std::to_string(stamp) + "_" + std::to_string(counter++));
            fs::create_directories(path_);
        }
        ~TempDir() { std::error_code ec; fs::remove_all(path_, ec); }

        const fs::path& path() const { return path_; }
        fs::path operator/(const std::string& s) const { return path_ / s; }

    private:
        fs::path path_;
    };

    inline void WriteFile(const fs::path& p, const std::string& content)
    {
        if (p.parent_path() != fs::path()) {
            fs::create_directories(p.parent_path());
        }
        std::ofstream out(p, std::ios::binary);
        out << content;
    }

    inline void WriteBytes(const fs::path& p, const std::vector<uint8_t>& bytes)
    {
        fs::create_directories(p.parent_path());
        std::ofstream out(p, std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytes.data()), (std::streamsize)bytes.size());
    }

    inline std::string ReadFile(const fs::path& p)
    {
        std::ifstream in(p, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(in)), {});
    }
}

// Gives each test a fresh, current Scene and registered serializers.
class SceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Vortx::SerializerLoader::Reset();
        Vortx::SceneManager::current = std::make_unique<Vortx::Scene>();
    }

    void TearDown() override { Vortx::SceneManager::current.reset(); }

    Vortx::Scene& scene() { return *Vortx::SceneManager::current; }

    Vortx::GameObject Make(const std::string& name = "")
    {
        Vortx::GameObject go = scene().CreateGameObject();
        if (!name.empty()) go.NameSet(name);
        return go;
    }

    static Vortx::Transform Tr(Vortx::GameObject go) { return go.GetComponent<Vortx::Transform>(); }

    std::optional<Vortx::GameObject> Find(const std::string& name)
    {
        for (auto& kv : scene().gameObjectsRegister)
        {
            Vortx::GameObject go = kv.second;
            if (go.NameGet() == name) return go;
        }
        return std::nullopt;
    }
};

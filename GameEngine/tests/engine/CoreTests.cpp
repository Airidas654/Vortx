#include "TestUtils.h"
#include "Core/DynamicLibrary.h"
#include "Core/WindowManager.h"
#include "SimpleAPI/InputManager.h"
#include "SimpleAPI/KeyCode.h"

using namespace Vortx;
using namespace TestUtils;

// --------------------------- DynamicLibrary ---------------------------

TEST(DynamicLibrary, DefaultIsNotLoaded)
{
    DynamicLibrary lib;
    EXPECT_FALSE(lib.IsLoaded());
    EXPECT_EQ(lib.GetSymbol<void*>("anything"), nullptr);
    lib.Unload();   // no-op, must not crash
}

TEST(DynamicLibrary, LoadingMissingLibraryFails)
{
    DynamicLibrary lib;
    EXPECT_FALSE(lib.Load("definitely_not_a_real_library_12345"));
    EXPECT_FALSE(lib.IsLoaded());
}

TEST(DynamicLibrary, LoadsSystemLibraryAndResolvesSymbol)
{
#ifdef _WIN32
    const char* name = "kernel32.dll"; const char* sym = "GetTickCount";
#elif __APPLE__
    const char* name = "libSystem.dylib"; const char* sym = "cos";
#else
    const char* name = "libm.so.6"; const char* sym = "cos";
#endif
    DynamicLibrary lib;
    ASSERT_TRUE(lib.Load(name));
    EXPECT_TRUE(lib.IsLoaded());
    EXPECT_NE(lib.GetSymbol<void*>(sym), nullptr);
    EXPECT_EQ(lib.GetSymbol<void*>("no_such_symbol_xyz"), nullptr);

    lib.Unload();
    EXPECT_FALSE(lib.IsLoaded());
    EXPECT_EQ(lib.GetSymbol<void*>(sym), nullptr);
}

// ---------------------- WindowManager (no window) ----------------------
// Only the parts that work without glfwInit()/a display.

TEST(WindowManager, NoWindowDefaults)
{
    EXPECT_FALSE(WindowManager::IsValid());
    EXPECT_EQ(WindowManager::GetHandle(), nullptr);
    EXPECT_FALSE(WindowManager::ShouldClose());
    EXPECT_EQ(WindowManager::GetWidth(), 0u);
    EXPECT_EQ(WindowManager::GetHeight(), 0u);
    EXPECT_EQ(WindowManager::GetFramebufferWidth(), 0u);
    EXPECT_EQ(WindowManager::GetFramebufferHeight(), 0u);
    EXPECT_EQ(WindowManager::GetMode(), WindowMode::Windowed);
    EXPECT_FALSE(WindowManager::IsFullscreen());
    EXPECT_FALSE(WindowManager::IsMinimized());
    EXPECT_TRUE(WindowManager::IsFocused());
}

TEST(WindowManager, ContentScaleAndPositionDefaultWithoutWindow)
{
    float sx = 0, sy = 0;
    WindowManager::GetContentScale(sx, sy);
    EXPECT_FLOAT_EQ(sx, 1.0f);
    EXPECT_FLOAT_EQ(sy, 1.0f);

    int x = 5, y = 5;
    WindowManager::GetPosition(x, y);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);
}

TEST(WindowManager, WindowOperationsWithoutWindowAreSafe)
{
    WindowManager::RequestClose();
    WindowManager::CancelClose();
    WindowManager::SetSize(10, 10);
    WindowManager::SetPosition(1, 1);
    WindowManager::SetTitle("x");
    WindowManager::SetCursorMode(CursorMode::Hidden);
    WindowManager::Destroy();
    SUCCEED();
}

// ------------------- InputManager (no window / no GLFW) -------------------

TEST(InputManager, OutOfRangeQueriesReturnFalse)
{
    EXPECT_FALSE(InputManager::GetKey(-1));
    EXPECT_FALSE(InputManager::GetKey(100000));
    EXPECT_FALSE(InputManager::GetKeyDown(100000));
    EXPECT_FALSE(InputManager::GetKeyUp(-5));
    EXPECT_FALSE(InputManager::GetMouseButton(99));
    EXPECT_FALSE(InputManager::GetMouseButtonDown(-1));
    EXPECT_FALSE(InputManager::GetMouseButtonUp(99));
    EXPECT_FALSE(InputManager::GetGamepadButton(0, 99));
    EXPECT_FALSE(InputManager::GetGamepadButtonDown(99, 0));
    EXPECT_FLOAT_EQ(InputManager::GetGamepadAxis(99, 0), 0.0f);
    EXPECT_FLOAT_EQ(InputManager::GetGamepadAxis(0, -1), 0.0f);
}

TEST(InputManager, IdleKeysAreNotPressed)
{
    EXPECT_FALSE(InputManager::GetKey(KeyCode::KEY_SPACE));
    EXPECT_FALSE(InputManager::GetKeyDown(KeyCode::KEY_A));
    EXPECT_FALSE(InputManager::AnyKeyPressed());
}

TEST(InputManager, UpdateWithoutWindowIsNoOp)
{
    InputManager::Update();
    SUCCEED();
}

TEST(InputManager, NoGamepadConnected)
{
    EXPECT_FALSE(InputManager::IsGamepadConnected(-1));
    EXPECT_FALSE(InputManager::IsGamepadConnected(99));
    EXPECT_EQ(InputManager::GetGamepadName(99), "");
}

TEST(InputManager, RegisterAndQueryBinding)
{
    InputManager::RegisterBinding("t_jump", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_SPACE));
    const InputKey* b = InputManager::GetBinding("t_jump");
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->name, "t_jump");
    EXPECT_EQ(b->code, KeyCode::KEY_SPACE);
    EXPECT_EQ(b->device, InputDeviceType::Keyboard);

    EXPECT_FALSE(InputManager::GetPressed("t_jump"));
    EXPECT_FALSE(InputManager::GetPressedDown("t_jump"));
    EXPECT_FALSE(InputManager::GetPressedUp("t_jump"));
}

TEST(InputManager, UnknownBindingIsNotPressed)
{
    EXPECT_EQ(InputManager::GetBinding("t_missing"), nullptr);
    EXPECT_FALSE(InputManager::GetPressed("t_missing"));
    EXPECT_FALSE(InputManager::GetPressedDown("t_missing"));
    EXPECT_FALSE(InputManager::GetPressedUp("t_missing"));
}

TEST(InputManager, RegisterDoesNotOverwriteExistingBinding)
{
    InputManager::RegisterBinding("t_keep", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_A));
    InputManager::SetBinding("t_keep", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_B));
    InputManager::RegisterBinding("t_keep", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_C));
    EXPECT_EQ(InputManager::GetBinding("t_keep")->code, KeyCode::KEY_B);
}

TEST(InputManager, ResetBindingsRestoresDefaults)
{
    InputManager::RegisterBinding("t_reset", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_A));
    InputManager::SetBinding("t_reset", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_Z));
    InputManager::ResetBindingsToDefault();
    EXPECT_EQ(InputManager::GetBinding("t_reset")->code, KeyCode::KEY_A);
}

TEST(InputManager, SaveAndLoadBindingsRoundTrip)
{
    TempDir tmp;
    auto path = (tmp / "bindings.json").string();

    InputKey axis("", InputDeviceType::GamepadAxis, 2);
    axis.axisPositive = false;
    axis.axisThreshold = 0.25f;
    InputManager::SetBinding("t_axis", axis);
    InputManager::SetBinding("t_fire", InputKey("", InputDeviceType::MouseButton, 1));

    ASSERT_TRUE(InputManager::SaveBindings(path));

    InputManager::SetBinding("t_axis", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_A));
    InputManager::SetBinding("t_fire", InputKey("", InputDeviceType::Keyboard, KeyCode::KEY_B));

    ASSERT_TRUE(InputManager::LoadBindings(path));

    const InputKey* a = InputManager::GetBinding("t_axis");
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->device, InputDeviceType::GamepadAxis);
    EXPECT_EQ(a->code, 2);
    EXPECT_FALSE(a->axisPositive);
    EXPECT_FLOAT_EQ(a->axisThreshold, 0.25f);

    const InputKey* f = InputManager::GetBinding("t_fire");
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->device, InputDeviceType::MouseButton);
    EXPECT_EQ(f->code, 1);
}

TEST(InputManager, SaveToInvalidPathFails)
{
    TempDir tmp;
    EXPECT_FALSE(InputManager::SaveBindings((tmp / "no_such_dir" / "b.json").string()));
}

TEST(InputManager, LoadBindingsFailureModes)
{
    TempDir tmp;
    EXPECT_FALSE(InputManager::LoadBindings((tmp / "missing.json").string()));

    WriteFile(tmp / "bad.json", "{ nope");
    EXPECT_FALSE(InputManager::LoadBindings((tmp / "bad.json").string()));

    WriteFile(tmp / "nokey.json", R"({"Other":[]})");
    EXPECT_FALSE(InputManager::LoadBindings((tmp / "nokey.json").string()));
}

TEST(InputManager, LoadSkipsEntriesWithoutName)
{
    TempDir tmp;
    WriteFile(tmp / "b.json", R"({"Bindings":[{"Device":"Keyboard","Code":65},{"Name":"t_named","Code":66}]})");
    ASSERT_TRUE(InputManager::LoadBindings((tmp / "b.json").string()));
    EXPECT_NE(InputManager::GetBinding("t_named"), nullptr);
    EXPECT_EQ(InputManager::GetBinding("t_named")->device, InputDeviceType::Keyboard);
}

TEST(InputManager, MousePositionHonoursViewportOverride)
{
    InputManager::_SetViewportOverride(true, { 10, 20 }, { 100, 100 });
    EXPECT_VEC2_NEAR(InputManager::GetMousePosition(), glm::vec2(-10, -20), 1e-4f);
    InputManager::_SetViewportOverride(false, { 0, 0 }, { 0, 0 });
    EXPECT_VEC2_NEAR(InputManager::GetMousePosition(), glm::vec2(0, 0), 1e-4f);
}

TEST(InputManager, MouseWorldPositionIsZeroWithoutViewport)
{
    InputManager::_SetViewportOverride(false, { 0, 0 }, { 0, 0 });
    EXPECT_VEC2_NEAR(InputManager::GetMouseWorldPosition(), glm::vec2(0, 0), 1e-6f);
}

TEST(InputManager, DeltasDefaultToZero)
{
    EXPECT_VEC2_NEAR(InputManager::GetMouseDelta(), glm::vec2(0, 0), 1e-6f);
    EXPECT_VEC2_NEAR(InputManager::GetScrollDelta(), glm::vec2(0, 0), 1e-6f);
}

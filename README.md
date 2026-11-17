# Vortx

**Vortx** is an open-source 2D game engine and editor written in modern C++. It pairs a data-oriented ECS core with a Unity-style editor (docking panels, hierarchy, inspector, scene/game views, gizmos, play mode) and lets you write gameplay in C++ with hot-reloadable scripts.

> ⚠️ Vortx is under active development. APIs and file formats may change between commits.

---

## Design philosophy

Vortx is built around two goals that usually pull in opposite directions: **performance** and **ease of use**.

- **ECS under the hood for performance.** All game state lives in plain data components (`TransformData`, `SpriteRendererData`, `RigidbodyData`, ...) stored in [EnTT](https://github.com/skypjack/entt) pools. Systems such as sprite submission, physics and transform updates iterate over these tightly packed arrays, which is cache-friendly and scales well to many objects.
- **An OOP-style API on top for easier programming.** Instead of working with the registry directly, you use familiar `GameObject`, `Transform`, `Rigidbody`, `Camera`, ... wrappers (the `SimpleAPI/` folder). They are thin handles over an entity, so you get readable code like `transform.Translate(...)` or `rigidbody.ApplyImpulse(...)` without giving up the data-oriented storage.

In short: write gameplay the object-oriented way, and let the ECS keep it fast.

---

## Features

### Editor
- Dockable ImGui workspace: **Scene**, **Game**, **Hierarchy**, **Properties**, **Assets** and **Console** panels
- **Project Hub** to create, add and open projects
- Scene-view camera (WASD / right-drag pan, scroll zoom) and click-to-pick selection
- **Move / Rotate / Scale gizmos** in Local or World space (hotkeys `1` / `2` / `3`)
- Hierarchy with drag-and-drop re-parenting and re-ordering
- Properties panel with an "Add Component" search popup (components and scripts)
- **Play / Pause / Step** with scene snapshot and restore on exit
- Asset browser with texture thumbnails, drag-and-drop assets into components, and per-asset import settings
- Built-in console that captures `std::cout` / `std::cerr`, with a select/copy mode
- Optional **auto-compile** of game code before entering play mode

### Engine
- **ECS** built on [EnTT](https://github.com/skypjack/entt), wrapped in a simple `GameObject` / `Transform` / component API
- **Transform hierarchy** with parent/child relationships, lazy dirty-flag world matrices, and cached world position/rotation/scale
- **Batched 2D renderer** (OpenGL 4.6 core) with layer and order-in-layer sorting, up to 16 texture slots per batch, and render targets for the editor views
- Renderer sits behind an `IRenderDevice` interface, so other graphics backends can be added
- **2D physics**: rigid bodies (static / kinematic / dynamic), box and circle colliders, SAT with two-point clipped manifolds, warm-started iterative velocity solver with friction and restitution, positional correction, sleeping, triggers, collision filtering (category / mask / group), and enter / stay / exit events
- **Asset database** with GUIDs, `.meta` files, ref-counted loading, scene dependency tracking, and scene preloading. Texture, audio (`.wav`) and font loaders are included
- **Input manager** with keyboard, mouse, gamepad, scroll, and named, rebindable actions that can be saved and loaded as JSON
- **Gizmos API** (`DrawLine`, `DrawBox`, `DrawCircle`, `DrawRay`) usable from scripts via `OnDrawGizmos` / `OnDrawGizmosSelected`
- **Scene serialization** to CBOR with per-component read/write registration
- `WindowManager` covering windowed, fullscreen and borderless modes, vsync, cursor modes and window callbacks

### Scripting
- Gameplay is written in **C++** and compiled into a `Game` shared library per project
- The editor builds your project with CMake and **hot-reloads** the library without restarting
- Scripts derive from `ScriptBase<T>` and are registered with a single macro
- Lifecycle callbacks: `Start`, `Update`, `FixedUpdate`, `Destroy`, `OnDrawGizmos`, `OnDrawGizmosSelected`

---

## Architecture

```
┌────────────────────────┐        loads at runtime        ┌──────────────────────────┐
│   Editor executable    │ ─────────────────────────────▶ │  GameEngineLib (.dll/.so)│
│ (ImGui, GLFW, glad)    │        GetEngine()             │  ECS, renderer, physics, │
│ main.cpp, Editor*, ... │                                │  assets, input, scenes   │
└───────────┬────────────┘                                └──────────────────────────┘
            │ cmake --build + copy Game.dll → GameLoaded.dll
            ▼
┌────────────────────────┐
│ Your project's Game.dll│  scripts registered via REGISTER_SCRIPT
└────────────────────────┘
```

---

## Getting started

### Requirements
- A C++17 (or newer) compiler
- CMake
- OpenGL 4.6 capable GPU and drivers

> **Platform note:** Vortx is developed on **Windows using Visual Studio**. Project build and hot reload look for `Game.dll`, `.pdb` files and a bundled `CMake/bin/cmake.exe`.

### Build the engine and editor

**Windows (Visual Studio, recommended)**

```bash
git clone https://github.com/<your-username>/<your-repo>.git
cd <your-repo>
cmake -S . -B build -G "Visual Studio 17 2022"
```

Then open the generated solution in `build/` with Visual Studio, select **Debug** or **Release**, and build. Alternatively, open the folder directly in Visual Studio and use its built-in CMake support.

Adjust the commands and the generator name to match your repository's top-level `CMakeLists.txt` and your Visual Studio version.

The editor expects, next to its executable:

| Path | Purpose |
|------|---------|
| `GameEngineLib.dll` / `.so` / `.dylib` | The engine library, loaded at startup |
| `EngineSDK/` | Engine headers used when compiling game projects |
| `Assets/ProjectTemplate/` | Template copied when you create a new project |
| `config.json` | Created on first run; set `CMakePath` to use your own CMake (empty = bundled `CMake/bin/cmake.exe`) |

### Create your first project
1. Launch the editor. The **Project Hub** appears.
2. Click **Create Project** and choose a folder name, or **Add Existing Project** and pick a `.vortx` file.
3. Open the project. The editor generates `EnginePaths.cmake` and starts compiling your game library automatically.
4. Create objects with **Scene → Create Empty**, add components in **Properties**, and drag textures from **Assets** onto a Sprite Renderer.
5. Press **▶** in the toolbar to enter play mode. **‖** pauses and **>|** steps one frame while paused.

---

## Writing scripts

```cpp
#include "GameEngineIncludes.h"

using namespace Vortx;

class PlayerController : public ScriptBase<PlayerController>
{
public:
    float speed = 5.0f;

    void Update() override
    {
        Transform t = gameObject.GetComponent<Transform>();

        glm::vec2 dir(0.0f);
        if (InputManager::GetKey(KeyCode::KEY_A)) dir.x -= 1.0f;
        if (InputManager::GetKey(KeyCode::KEY_D)) dir.x += 1.0f;
        if (InputManager::GetKey(KeyCode::KEY_W)) dir.y += 1.0f;
        if (InputManager::GetKey(KeyCode::KEY_S)) dir.y -= 1.0f;

        t.Translate(dir * speed * Time::deltaTime);
    }

    void OnDrawGizmosSelected() override
    {
        Gizmos::color = glm::u8vec4(255, 200, 0, 255);
        Gizmos::DrawCircle(gameObject.GetComponent<Transform>().Position(), 1.0f);
    }
};

REGISTER_SCRIPT(PlayerController);
```

After saving, press **Compile** (`File → Compile`) or enable **Auto Compile**. The script appears under **Add Component → Scripts**.

---

## Project file layout

```
MyGame/
├── project.vortx          # JSON: project name, scene list, settings
├── CMakeLists.txt         # builds Game.dll against the engine SDK
├── EnginePaths.cmake      # auto-generated by the editor, do not edit
├── Assets/
│   ├── player.png
│   ├── player.png.meta    # GUID + import settings
│   └── Main.scene         # CBOR-serialized scene
│   └── Script.h           # Game script
```

Supported asset extensions: images (`.png .jpg .jpeg .bmp .tga`), audio (`.wav .ogg .mp3`), fonts (`.ttf .otf`), scenes (`.scene`) and scripts (`.h`).

---

## Editor controls

| Action | Control |
|--------|---------|
| Pan scene view | Right mouse drag, or `W` `A` `S` `D` |
| Zoom | Mouse wheel |
| Select object | Left click in the Scene view or Hierarchy |
| Move / Rotate / Scale gizmo | `1` / `2` / `3` or toolbar buttons `M` `R` `S` |
| Local / World gizmo space | Toolbar buttons `L` `W` |
| Re-parent / reorder | Drag in the Hierarchy (drop on the top or bottom edge to reorder, on the middle to parent) |
| Compile game code | `File → Compile` |

---

## Current status and roadmap

**Working:** editor workflow, ECS, sprite rendering, scene save/load, asset database, some of the physics (boxes and circles), input, gizmos, basic C++ script hot reload.

**Known gaps / ideas:**
- [ ] Physics queries: `Raycast`, `RaycastAll`, `OverlapPoint`, `OverlapCircle`, `OverlapBox`
- [ ] Audio playback (`.wav` is decoded, but there is no mixer or `AudioSource` yet)
- [ ] Text rendering
- [ ] Animation (Tweening class)
- [ ] Object duplication in the Hierarchy
- [ ] Script field serialization and inspector editing
- [ ] Physics wake islands
- [ ] Linux and macOS editor support
- [ ] additional render backends
- [ ] Standalone game export
- [ ] Particle system
- [ ] Undo system

And probably more

---

## Contributing

Contributions, bug reports and ideas are welcome.

1. Fork the repository and create a feature branch
2. Keep changes focused and match the surrounding code style
3. Open a pull request describing what changed and why

### Suggestions and questions

Have an idea for a new feature, or a suggestion for a better structure or architecture? Please use the **[Discussions](../../discussions)** tab rather than opening an issue. Issues are best kept for concrete bugs and tracked tasks.

## License

Vortx is released under the **MIT License**. See the [LICENSE](LICENSE) file for details.

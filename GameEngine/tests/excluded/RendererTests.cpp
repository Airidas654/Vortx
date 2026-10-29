#include "TestUtils.h"
#include "Assets/TextureAsset.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Components/ComponentTransform.h"
#include "Core/AssetMeta.h"
#include "Renderer/GameViewport.h"
#include "Renderer/IRenderDevice.h"
#include "Renderer/SpriteRendererSystem.h"
#include "SimpleAPI/Renderer.h"
#include "SimpleAPI/SpriteRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Vortx;
using namespace TestUtils;

namespace
{
    // Records every call so the CPU-side batching logic can be verified without OpenGL.
    class FakeRenderDevice : public IRenderDevice
    {
    public:
        struct Batch
        {
            std::vector<QuadVertex> verts;
            uint32_t quadCount = 0;
            std::vector<TextureHandle> textures;
        };

        struct TexInfo { uint32_t w, h; TextureFilter filter; TextureWrap wrap; };

        bool initResult = true;
        int shutdownCalls = 0, shadersCreated = 0, shadersDestroyed = 0;
        int texturesDestroyed = 0, targetsDestroyed = 0, resizeTargetCalls = 0, beginFrames = 0, endFrames = 0;
        std::vector<TexInfo> createdTextures;
        std::vector<Batch> batches;
        glm::vec4 lastClear{ -1.0f };
        glm::mat4 lastViewProjection{ 0.0f };
        RenderTargetHandle lastBound{ 12345 };
        uint32_t lastResizeW = 0, lastResizeH = 0;
        glm::uvec2 resizeEvent{ 0, 0 };

        uint32_t nextTex = 100, nextRT = 1;

        bool Init(GLFWwindow*) override { return initResult; }
        void Shutdown() override { shutdownCalls++; }
        void OnResize(uint32_t w, uint32_t h) override { resizeEvent = { w, h }; }
        void BeginFrame() override { beginFrames++; }
        void Clear(const glm::vec4& c) override { lastClear = c; }
        void SetViewProjection(const glm::mat4& m) override { lastViewProjection = m; }
        void EndFrame() override { endFrames++; }

        RenderTargetHandle CreateRenderTarget(uint32_t, uint32_t) override { return { nextRT++ }; }
        void ResizeRenderTarget(RenderTargetHandle, uint32_t w, uint32_t h) override { resizeTargetCalls++; lastResizeW = w; lastResizeH = h; }
        void DestroyRenderTarget(RenderTargetHandle) override { targetsDestroyed++; }
        TextureHandle GetRenderTargetTexture(RenderTargetHandle t) override { return { 900 + t.id }; }
        void BindRenderTarget(RenderTargetHandle t) override { lastBound = t; }

        TextureHandle CreateTexture(const TextureDesc& d) override
        {
            createdTextures.push_back({ d.width, d.height, d.filter, d.wrap });
            return { nextTex++ };
        }
        void DestroyTexture(TextureHandle) override { texturesDestroyed++; }
        void* GetNativeTextureHandle(TextureHandle t) override { return (void*)(intptr_t)t.id; }

        ShaderHandle CreateShader(const std::string&, const std::string&) override { shadersCreated++; return { 1 }; }
        void DestroyShader(ShaderHandle) override { shadersDestroyed++; }

        void SubmitBatch(const QuadVertex* v, uint32_t quads, const TextureHandle* tex, uint32_t texCount, ShaderHandle) override
        {
            Batch b;
            b.verts.assign(v, v + quads * 4);
            b.quadCount = quads;
            b.textures.assign(tex, tex + texCount);
            batches.push_back(std::move(b));
        }
    };

    std::vector<uint8_t> MakeBmp2x2()
    {
        std::vector<uint8_t> v;
        auto le = [&](uint32_t x, int n) { for (int i = 0; i < n; i++) v.push_back((x >> (8 * i)) & 0xFF); };
        v.push_back('B'); v.push_back('M');
        le(70, 4); le(0, 4); le(54, 4);
        le(40, 4); le(2, 4); le(2, 4); le(1, 2); le(24, 2);
        le(0, 4); le(16, 4); le(0, 4); le(0, 4); le(0, 4); le(0, 4);
        for (int row = 0; row < 2; row++)
        {
            for (int px = 0; px < 2; px++) { v.push_back(10); v.push_back(20); v.push_back(30); }
            v.push_back(0); v.push_back(0);   // row padding to 4 bytes
        }
        return v;
    }
}

class RendererTest : public SceneTest
{
protected:
    FakeRenderDevice* fake = nullptr;

    void SetUp() override
    {
        SceneTest::SetUp();
        auto dev = std::make_unique<FakeRenderDevice>();
        fake = dev.get();
        ASSERT_TRUE(Renderer::Init(std::move(dev), nullptr));
        Renderer::NewFrame();
    }

    void TearDown() override
    {
        Renderer::Shutdown();
        fake = nullptr;
        SceneTest::TearDown();
    }

    void Render(glm::vec4 clear = glm::vec4(0.0f)) { Renderer::RenderView(glm::mat4(1.0f), clear); }
};

// ------------------------------- init / lifecycle -------------------------------

TEST_F(RendererTest, InitCreatesShaderAndWhiteTexture)
{
    EXPECT_EQ(fake->shadersCreated, 1);
    ASSERT_EQ(fake->createdTextures.size(), 1u);
    EXPECT_EQ(fake->createdTextures[0].w, 1u);
    EXPECT_EQ(fake->createdTextures[0].h, 1u);
    EXPECT_NE(Renderer::GetDevice(), nullptr);
}

TEST(RendererInit, FailingDeviceInitReturnsFalse)
{
    auto dev = std::make_unique<FakeRenderDevice>();
    dev->initResult = false;
    EXPECT_FALSE(Renderer::Init(std::move(dev), nullptr));
    Renderer::Shutdown();
}

TEST_F(RendererTest, NewFrameBeginsDeviceFrameAndPresentEndsIt)
{
    int before = fake->beginFrames;
    Renderer::NewFrame();
    EXPECT_EQ(fake->beginFrames, before + 1);
    Renderer::Present();
    EXPECT_EQ(fake->endFrames, 1);
}

TEST_F(RendererTest, OnResizeForwardsToDevice)
{
    Renderer::OnResize(800, 600);
    EXPECT_EQ(fake->resizeEvent, glm::uvec2(800, 600));
}

TEST_F(RendererTest, RenderTargetAndTextureCallsForwardToDevice)
{
    RenderTargetHandle rt = Renderer::CreateRenderTarget(64, 64);
    EXPECT_TRUE(rt.IsValid());
    EXPECT_TRUE(Renderer::GetRenderTargetTexture(rt).IsValid());

    Renderer::ResizeRenderTarget(rt, 128, 32);
    EXPECT_EQ(fake->lastResizeW, 128u);
    EXPECT_EQ(fake->lastResizeH, 32u);

    Renderer::DestroyRenderTarget(rt);
    EXPECT_EQ(fake->targetsDestroyed, 1);

    TextureDesc d; d.width = 4; d.height = 4;
    TextureHandle t = Renderer::CreateTexture(d);
    EXPECT_TRUE(t.IsValid());
    EXPECT_EQ(Renderer::GetNativeTextureHandle(t), (void*)(intptr_t)t.id);
    Renderer::DestroyTexture(t);
    EXPECT_EQ(fake->texturesDestroyed, 1);
}

// ------------------------------- drawing / batching -------------------------------

TEST_F(RendererTest, EmptyFrameSubmitsNothing)
{
    Render();
    EXPECT_TRUE(fake->batches.empty());
}

TEST_F(RendererTest, RenderViewBindsTargetClearsAndSetsMatrix)
{
    glm::mat4 vp = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    RenderTargetHandle rt = Renderer::CreateRenderTarget(8, 8);

    Renderer::RenderView(vp, glm::vec4(0.1f, 0.2f, 0.3f, 1.0f), rt);

    EXPECT_EQ(fake->lastBound.id, rt.id);
    EXPECT_EQ(fake->lastClear, glm::vec4(0.1f, 0.2f, 0.3f, 1.0f));
    EXPECT_EQ(fake->lastViewProjection, vp);
}

TEST_F(RendererTest, ClearAndBindBackbuffer)
{
    Renderer::Clear(glm::vec4(1, 0, 0, 1));
    EXPECT_EQ(fake->lastClear, glm::vec4(1, 0, 0, 1));
    Renderer::BindBackbuffer();
    EXPECT_EQ(fake->lastBound.id, 0u);
}

TEST_F(RendererTest, QuadVerticesFollowTransform)
{
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(1, 1, 0));
    t = glm::scale(t, glm::vec3(2, 2, 1));
    Renderer::DrawQuad(t, glm::vec4(1, 0, 0, 1));
    Render();

    ASSERT_EQ(fake->batches.size(), 1u);
    auto& v = fake->batches[0].verts;
    ASSERT_EQ(v.size(), 4u);
    EXPECT_NEAR(v[0].position.x, 0.0f, 1e-5f); EXPECT_NEAR(v[0].position.y, 0.0f, 1e-5f);
    EXPECT_NEAR(v[2].position.x, 2.0f, 1e-5f); EXPECT_NEAR(v[2].position.y, 2.0f, 1e-5f);
    EXPECT_EQ(v[0].color, glm::vec4(1, 0, 0, 1));
    EXPECT_EQ(v[2].uv, glm::vec2(1, 1));
    EXPECT_FLOAT_EQ(v[0].texIndex, 0.0f);   // white texture slot
}

TEST_F(RendererTest, PositionSizeOverloadBuildsMatchingQuad)
{
    Renderer::DrawQuad(glm::vec2(3, 4), glm::vec2(2, 2), glm::vec1(0), glm::vec4(1));
    Render();
    auto& v = fake->batches.at(0).verts;
    EXPECT_NEAR(v[0].position.x, 2.0f, 1e-5f); EXPECT_NEAR(v[0].position.y, 3.0f, 1e-5f);
    EXPECT_NEAR(v[2].position.x, 4.0f, 1e-5f); EXPECT_NEAR(v[2].position.y, 5.0f, 1e-5f);
}

TEST_F(RendererTest, QuadsWithSameTextureShareOneBatch)
{
    for (int i = 0; i < 5; i++) Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1));
    Render();
    ASSERT_EQ(fake->batches.size(), 1u);
    EXPECT_EQ(fake->batches[0].quadCount, 5u);
    EXPECT_EQ(fake->batches[0].textures.size(), 1u);
}

TEST_F(RendererTest, DifferentTexturesGetDifferentSlotsInOneBatch)
{
    TextureHandle tex{ 500 };
    Renderer::DrawSprite(glm::mat4(1.0f), tex, glm::vec4(1));
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1));
    Render();

    ASSERT_EQ(fake->batches.size(), 1u);
    auto& b = fake->batches[0];
    ASSERT_EQ(b.textures.size(), 2u);
    EXPECT_EQ(b.textures[1].id, 500u);
    EXPECT_FLOAT_EQ(b.verts[0].texIndex, 1.0f);   // sprite
    EXPECT_FLOAT_EQ(b.verts[4].texIndex, 0.0f);   // quad
}

TEST_F(RendererTest, ExceedingTextureSlotsSplitsBatches)
{
    for (uint32_t i = 0; i < 20; i++)
        Renderer::DrawSprite(glm::mat4(1.0f), TextureHandle{ 1000 + i }, glm::vec4(1));
    Render();

    ASSERT_EQ(fake->batches.size(), 2u);
    EXPECT_EQ(fake->batches[0].quadCount + fake->batches[1].quadCount, 20u);
    EXPECT_LE(fake->batches[0].textures.size(), 16u);
}

TEST_F(RendererTest, SortsByLayerThenOrderInLayer)
{
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1, 0, 0, 1), 5, 0);   // red,   layer 5
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(0, 1, 0, 1), 1, 9);   // green, layer 1 order 9
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(0, 0, 1, 1), 1, 2);   // blue,  layer 1 order 2
    Render();

    auto& v = fake->batches.at(0).verts;
    EXPECT_EQ(v[0].color, glm::vec4(0, 0, 1, 1));
    EXPECT_EQ(v[4].color, glm::vec4(0, 1, 0, 1));
    EXPECT_EQ(v[8].color, glm::vec4(1, 0, 0, 1));
}

TEST_F(RendererTest, EqualSortKeysKeepSubmissionOrder)
{
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1, 0, 0, 1));
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(0, 1, 0, 1));
    Render();
    auto& v = fake->batches.at(0).verts;
    EXPECT_EQ(v[0].color, glm::vec4(1, 0, 0, 1));
    EXPECT_EQ(v[4].color, glm::vec4(0, 1, 0, 1));
}

TEST_F(RendererTest, NewFrameDiscardsQueuedDrawCommands)
{
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1));
    Renderer::NewFrame();
    Render();
    EXPECT_TRUE(fake->batches.empty());
}

TEST_F(RendererTest, SameViewCanBeRenderedTwiceFromOneFrameQueue)
{
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1));
    Render();
    Render();
    EXPECT_EQ(fake->batches.size(), 2u);   // scene + game view share the queue
}

// ------------------------------ SpriteRenderSystem ------------------------------

TEST_F(RendererTest, SpriteSystemSubmitsQuadPerSpriteRenderer)
{
    GameObject go = Make();
    Tr(go).SetPosition({ 2, 0 });
    SpriteRenderer sr = go.AddComponent<SpriteRenderer>();
    sr.Data().color = glm::vec4(1, 0, 0, 1);
    sr.Data().layer = 3;
    scene().UpdateTransforms();

    SpriteRenderSystem::Submit();
    Render();

    ASSERT_EQ(fake->batches.size(), 1u);
    auto& v = fake->batches[0].verts;
    EXPECT_EQ(v[0].color, glm::vec4(1, 0, 0, 1));
    EXPECT_NEAR(v[0].position.x, 1.5f, 1e-5f);
}

TEST_F(RendererTest, SpriteWithUnloadedTextureFallsBackToWhiteQuad)
{
    GameObject go = Make();
    go.AddComponent<SpriteRenderer>().SetTexture(AssetID(99, 99));
    scene().UpdateTransforms();

    SpriteRenderSystem::Submit();
    Render();

    ASSERT_EQ(fake->batches.size(), 1u);
    EXPECT_FLOAT_EQ(fake->batches[0].verts[0].texIndex, 0.0f);
}

TEST_F(RendererTest, SpriteSystemWithoutSceneIsSafe)
{
    SceneManager::current.reset();
    SpriteRenderSystem::Submit();
    SUCCEED();
}

// -------------------------------- GameViewport --------------------------------

TEST_F(RendererTest, EmbeddedViewportLifecycle)
{
    EXPECT_EQ(GameViewport::GetMode(), GameViewportMode::Uninitialized);

    GameViewport::InitEmbedded(640, 360);
    EXPECT_EQ(GameViewport::GetMode(), GameViewportMode::Embedded);
    EXPECT_EQ(GameViewport::GetWidth(), 640u);
    EXPECT_EQ(GameViewport::GetHeight(), 360u);
    EXPECT_NEAR(GameViewport::GetAspect(), 640.0f / 360.0f, 1e-5f);
    EXPECT_TRUE(GameViewport::IsReady());
    EXPECT_TRUE(GameViewport::GetRenderTarget().IsValid());
    EXPECT_TRUE(GameViewport::GetColorTexture().IsValid());

    GameViewport::Shutdown();
    EXPECT_EQ(GameViewport::GetMode(), GameViewportMode::Uninitialized);
    EXPECT_EQ(fake->targetsDestroyed, 1);
    EXPECT_FALSE(GameViewport::IsReady());
    EXPECT_FALSE(GameViewport::GetRenderTarget().IsValid());
    EXPECT_FLOAT_EQ(GameViewport::GetAspect(), 1.0f);
}

TEST_F(RendererTest, EmbeddedViewportResizeOnlyWhenSizeChanges)
{
    GameViewport::InitEmbedded(100, 100);

    GameViewport::SetEmbeddedSize(100, 100);
    EXPECT_EQ(fake->resizeTargetCalls, 0);

    GameViewport::SetEmbeddedSize(200, 50);
    EXPECT_EQ(fake->resizeTargetCalls, 1);
    EXPECT_EQ(GameViewport::GetWidth(), 200u);
    EXPECT_EQ(GameViewport::GetHeight(), 50u);

    GameViewport::Shutdown();
}

TEST_F(RendererTest, EmbeddedViewportZeroSizeIsNotReadyAndDoesNotResizeTarget)
{
    GameViewport::InitEmbedded(100, 100);
    GameViewport::SetEmbeddedSize(0, 50);
    EXPECT_FALSE(GameViewport::IsReady());
    EXPECT_EQ(fake->resizeTargetCalls, 0);
    GameViewport::Shutdown();
}

TEST_F(RendererTest, ViewportRenderDrawsIntoItsTarget)
{
    GameViewport::InitEmbedded(64, 64);
    Renderer::DrawQuad(glm::mat4(1.0f), glm::vec4(1));

    GameViewport::Render(glm::mat4(1.0f), glm::vec4(0.5f));

    EXPECT_EQ(fake->lastBound.id, GameViewport::GetRenderTarget().id);
    EXPECT_EQ(fake->batches.size(), 1u);
    GameViewport::Shutdown();
}

TEST_F(RendererTest, SetEmbeddedSizeIgnoredWhenNotEmbedded)
{
    GameViewport::SetEmbeddedSize(10, 10);
    EXPECT_EQ(GameViewport::GetWidth(), 0u);
}

// -------------------------------- TextureAsset --------------------------------

TEST_F(RendererTest, TextureAssetLoadsImageAndUsesImportSettings)
{
    TempDir tmp;
    WriteBytes(tmp / "t.bmp", MakeBmp2x2());

    AssetMeta meta;
    meta.importSettings["filterLinear"] = false;
    meta.importSettings["wrapRepeat"] = true;

    size_t before = fake->createdTextures.size();
    auto* tex = static_cast<TextureAsset*>(TextureAsset::Load((tmp / "t.bmp").string(), meta));
    ASSERT_NE(tex, nullptr);
    EXPECT_EQ(tex->width, 2);
    EXPECT_EQ(tex->height, 2);
    EXPECT_EQ(tex->channels, 4);
    EXPECT_TRUE(tex->handle.IsValid());

    ASSERT_EQ(fake->createdTextures.size(), before + 1);
    EXPECT_EQ(fake->createdTextures.back().filter, TextureFilter::Nearest);
    EXPECT_EQ(fake->createdTextures.back().wrap, TextureWrap::Repeat);

    int destroyedBefore = fake->texturesDestroyed;
    TextureAsset::Unload(tex);
    EXPECT_EQ(fake->texturesDestroyed, destroyedBefore + 1);
}

TEST_F(RendererTest, TextureAssetDefaultsToLinearClamp)
{
    TempDir tmp;
    WriteBytes(tmp / "t.bmp", MakeBmp2x2());

    auto* tex = static_cast<TextureAsset*>(TextureAsset::Load((tmp / "t.bmp").string(), AssetMeta{}));
    ASSERT_NE(tex, nullptr);
    EXPECT_EQ(fake->createdTextures.back().filter, TextureFilter::Linear);
    EXPECT_EQ(fake->createdTextures.back().wrap, TextureWrap::ClampToEdge);
    TextureAsset::Unload(tex);
}

TEST_F(RendererTest, TextureAssetMissingOrCorruptFileReturnsNull)
{
    TempDir tmp;
    EXPECT_EQ(TextureAsset::Load((tmp / "none.png").string(), AssetMeta{}), nullptr);
    WriteFile(tmp / "bad.png", "definitely not an image");
    EXPECT_EQ(TextureAsset::Load((tmp / "bad.png").string(), AssetMeta{}), nullptr);
}

// ------------------- asset-id based drawing / scene asset sync -------------------

#include "Core/AssetDatabase.h"

namespace
{
    void RegisterRealTextureLoader()
    {
        AssetDatabase::RegisterLoader(AssetType::Texture,
            [](const std::string& p, const AssetMeta& m) -> Asset* { return TextureAsset::Load(p, m); },
            [](Asset* a) { TextureAsset::Unload(static_cast<TextureAsset*>(a)); });
    }
}

TEST_F(RendererTest, DrawSpriteByAssetIdUsesLoadedTextureHandle)
{
    TempDir tmp;
    WriteBytes(tmp / "Assets" / "t.bmp", MakeBmp2x2());
    RegisterRealTextureLoader();
    AssetDatabase::Refresh((tmp / "Assets").string());

    AssetID id(0);
    ASSERT_TRUE(AssetDatabase::TryGetGuid((tmp / "Assets" / "t.bmp").string(), id));
    auto* tex = AssetDatabase::Acquire<TextureAsset>(id, "Test");
    ASSERT_NE(tex, nullptr);

    Renderer::DrawSprite(glm::mat4(1.0f), id);
    Render();

    ASSERT_EQ(fake->batches.size(), 1u);
    ASSERT_EQ(fake->batches[0].textures.size(), 2u);
    EXPECT_EQ(fake->batches[0].textures[1].id, tex->handle.id);

    AssetDatabase::Release(id, "Test");
    AssetDatabase::UnloadAll();
}

TEST_F(RendererTest, SpriteSystemUsesLoadedTextureAfterSceneAssetSync)
{
    TempDir tmp;
    WriteBytes(tmp / "Assets" / "t.bmp", MakeBmp2x2());
    RegisterRealTextureLoader();
    AssetDatabase::Refresh((tmp / "Assets").string());
    AssetID id(0);
    ASSERT_TRUE(AssetDatabase::TryGetGuid((tmp / "Assets" / "t.bmp").string(), id));

    GameObject go = Make();
    go.AddComponent<SpriteRenderer>().SetTexture(id);
    scene().SyncAssetsIfDirty();
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 1);          // scene owns a reference

    scene().UpdateTransforms();
    SpriteRenderSystem::Submit();
    Render();
    ASSERT_EQ(fake->batches.size(), 1u);
    EXPECT_EQ(fake->batches[0].textures.size(), 2u);        // white + sprite texture
    EXPECT_FLOAT_EQ(fake->batches[0].verts[0].texIndex, 1.0f);

    scene().DestroyGameObject(go);
    scene().SyncAssetsIfDirty();
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 0);           // released once unreferenced

    AssetDatabase::UnloadAll();
}

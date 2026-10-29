#include "TestUtils.h"
#include "Assets/AudioAsset.h"
#include "Assets/FontAsset.h"

using namespace Vortx;
using namespace TestUtils;

namespace
{
    void PushLE(std::vector<uint8_t>& v, uint32_t x, int bytes)
    {
        for (int i = 0; i < bytes; i++) v.push_back((uint8_t)((x >> (8 * i)) & 0xFF));
    }

    void PushTag(std::vector<uint8_t>& v, const char* tag) { v.insert(v.end(), tag, tag + 4); }

    std::vector<uint8_t> MakeWav(uint16_t channels, uint32_t rate, uint16_t bits, const std::vector<uint8_t>& pcm)
    {
        std::vector<uint8_t> v;
        PushTag(v, "RIFF");
        PushLE(v, 36 + (uint32_t)pcm.size(), 4);
        PushTag(v, "WAVE");
        PushTag(v, "fmt ");
        PushLE(v, 16, 4);
        PushLE(v, 1, 2);
        PushLE(v, channels, 2);
        PushLE(v, rate, 4);
        PushLE(v, rate * channels * bits / 8, 4);
        PushLE(v, channels * bits / 8, 2);
        PushLE(v, bits, 2);
        PushTag(v, "data");
        PushLE(v, (uint32_t)pcm.size(), 4);
        v.insert(v.end(), pcm.begin(), pcm.end());
        return v;
    }
}

// ----------------------------- AudioAsset -----------------------------

TEST(AudioAsset, LoadsPcmWav)
{
    TempDir tmp;
    std::vector<uint8_t> pcm = { 1, 2, 3, 4, 5, 6, 7, 8 };
    WriteBytes(tmp / "s.wav", MakeWav(2, 44100, 16, pcm));

    auto* a = static_cast<AudioAsset*>(AudioAsset::Load((tmp / "s.wav").string(), AssetMeta{}));
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->channels, 2);
    EXPECT_EQ(a->sampleRate, 44100u);
    EXPECT_EQ(a->bitsPerSample, 16);
    EXPECT_EQ(a->pcmData, pcm);
    AudioAsset::Unload(a);
}

TEST(AudioAsset, SkipsUnknownChunksBeforeData)
{
    TempDir tmp;
    auto wav = MakeWav(1, 8000, 8, { 9, 9, 9, 9 });
    // Insert a "LIST" chunk (4 bytes payload) right after the fmt chunk (offset 36).
    std::vector<uint8_t> extra;
    PushTag(extra, "LIST"); PushLE(extra, 4, 4); PushLE(extra, 0xDEADBEEF, 4);
    wav.insert(wav.begin() + 36, extra.begin(), extra.end());
    WriteBytes(tmp / "s.wav", wav);

    auto* a = static_cast<AudioAsset*>(AudioAsset::Load((tmp / "s.wav").string(), AssetMeta{}));
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->pcmData.size(), 4u);
    AudioAsset::Unload(a);
}

TEST(AudioAsset, RejectsNonWavExtension)
{
    TempDir tmp;
    WriteFile(tmp / "s.ogg", "OggS");
    EXPECT_EQ(AudioAsset::Load((tmp / "s.ogg").string(), AssetMeta{}), nullptr);
}

TEST(AudioAsset, RejectsGarbageWav)
{
    TempDir tmp;
    WriteFile(tmp / "bad.wav", "this is not a riff file");
    EXPECT_EQ(AudioAsset::Load((tmp / "bad.wav").string(), AssetMeta{}), nullptr);
}

TEST(AudioAsset, RejectsMissingFile)
{
    TempDir tmp;
    EXPECT_EQ(AudioAsset::Load((tmp / "missing.wav").string(), AssetMeta{}), nullptr);
}

TEST(AudioAsset, RejectsWavWithoutDataChunk)
{
    TempDir tmp;
    auto wav = MakeWav(1, 8000, 8, {});
    WriteBytes(tmp / "empty.wav", wav);
    EXPECT_EQ(AudioAsset::Load((tmp / "empty.wav").string(), AssetMeta{}), nullptr);
}

// ----------------------------- FontAsset -----------------------------

TEST(FontAsset, LoadsRawBytes)
{
    TempDir tmp;
    std::vector<uint8_t> bytes = { 0, 1, 2, 250, 251, 252 };
    WriteBytes(tmp / "f.ttf", bytes);

    auto* f = static_cast<FontAsset*>(FontAsset::Load((tmp / "f.ttf").string(), AssetMeta{}));
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->fileData, bytes);
    FontAsset::Unload(f);
}

TEST(FontAsset, MissingFileReturnsNull)
{
    TempDir tmp;
    EXPECT_EQ(FontAsset::Load((tmp / "nope.ttf").string(), AssetMeta{}), nullptr);
}

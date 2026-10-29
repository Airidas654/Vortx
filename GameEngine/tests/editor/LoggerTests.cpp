#include <gtest/gtest.h>
#include "Logger.h"

#include <sstream>
#include <thread>

using namespace Vortx;

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override { Logger::Clear(); }
    void TearDown() override { Logger::Clear(); }
};

TEST_F(LoggerTest, AddLogStoresTextAndLevel)
{
    Logger::AddLog("hello", LogLevel::Info);
    Logger::AddLog("boom", LogLevel::Error);

    ASSERT_EQ(Logger::GetLineCount(), 2);
    EXPECT_EQ(Logger::lines[0].text, "hello");
    EXPECT_EQ(Logger::lines[0].level, LogLevel::Info);
    EXPECT_EQ(Logger::lines[1].text, "boom");
    EXPECT_EQ(Logger::lines[1].level, LogLevel::Error);
}

TEST_F(LoggerTest, ClearRemovesAllLines)
{
    Logger::AddLog("x", LogLevel::Info);
    Logger::Clear();
    EXPECT_EQ(Logger::GetLineCount(), 0);
}

TEST_F(LoggerTest, AddLogIsThreadSafe)
{
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; t++)
        threads.emplace_back([] { for (int i = 0; i < 250; i++) Logger::AddLog("l", LogLevel::Info); });
    for (auto& th : threads) th.join();
    EXPECT_EQ(Logger::GetLineCount(), 1000);
}

TEST_F(LoggerTest, StreamBufSplitsOnNewlinesAndForwardsOriginal)
{
    std::stringbuf original;
    LoggerStreamBuf buf(&original, LogLevel::Info);
    std::ostream os(&buf);

    os << "first\nsecond\n";

    ASSERT_EQ(Logger::GetLineCount(), 2);
    EXPECT_EQ(Logger::lines[0].text, "first");
    EXPECT_EQ(Logger::lines[1].text, "second");
    EXPECT_EQ(original.str(), "first\nsecond\n");
}

TEST_F(LoggerTest, StreamBufHoldsPartialLineUntilNewline)
{
    LoggerStreamBuf buf(nullptr, LogLevel::Info);
    std::ostream os(&buf);

    os << "partial";
    EXPECT_EQ(Logger::GetLineCount(), 0);
    os << " line\n";
    ASSERT_EQ(Logger::GetLineCount(), 1);
    EXPECT_EQ(Logger::lines[0].text, "partial line");
}

TEST_F(LoggerTest, StreamBufSkipsEmptyLines)
{
    LoggerStreamBuf buf(nullptr, LogLevel::Info);
    std::ostream os(&buf);
    os << "\n\nreal\n\n";
    ASSERT_EQ(Logger::GetLineCount(), 1);
    EXPECT_EQ(Logger::lines[0].text, "real");
}

TEST_F(LoggerTest, StreamBufTagsEntriesWithItsLevel)
{
    LoggerStreamBuf buf(nullptr, LogLevel::Error);
    std::ostream os(&buf);
    os << "bad\n";
    ASSERT_EQ(Logger::GetLineCount(), 1);
    EXPECT_EQ(Logger::lines[0].level, LogLevel::Error);
}

TEST_F(LoggerTest, EndlWorksThroughStreamBuf)
{
    LoggerStreamBuf buf(nullptr, LogLevel::Info);
    std::ostream os(&buf);
    os << "with endl" << std::endl;
    ASSERT_EQ(Logger::GetLineCount(), 1);
    EXPECT_EQ(Logger::lines[0].text, "with endl");
}

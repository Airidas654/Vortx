#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <streambuf>

namespace Vortx
{
    enum class LogLevel { Info, Error };

    struct LogEntry
    {
        std::string text;
        LogLevel level;
    };

    class Logger
    {
    public:
        static std::vector<LogEntry> lines;
        static std::mutex mutex;

        static int GetLineCount();
        static void AddLog(const std::string& line, LogLevel level);
        static void Clear();
    };

    class LoggerStreamBuf : public std::streambuf
    {
    public:
        LoggerStreamBuf(std::streambuf* original, LogLevel level);

    protected:
        int overflow(int c) override;

    private:
        std::streambuf* original;
        std::string buffer;
        LogLevel level;
    };
}
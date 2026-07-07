#include "Logger.h"

namespace Vortx
{
    std::vector<LogEntry> Logger::lines;
    std::mutex Logger::mutex;

    void Logger::AddLog(const std::string& line, LogLevel level)
    {
        std::lock_guard<std::mutex> lock(mutex);
        lines.push_back({ line, level });
    }

    void Logger::Clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        lines.clear();
    }

    int Logger::GetLineCount()
    {
        return lines.size();
    }

    LoggerStreamBuf::LoggerStreamBuf(std::streambuf* original, LogLevel level) : original(original), level(level) {}

    int LoggerStreamBuf::overflow(int c)
    {
        if (original) original->sputc((char)c);

        if (c == '\n')
        {
            if (!buffer.empty())
            {
                Logger::AddLog(buffer, level);
                buffer.clear();
            }
        }
        else if (c != std::char_traits<char>::eof())
        {
            buffer += (char)c;
        }

        return c;
    }
}
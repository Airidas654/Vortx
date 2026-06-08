#pragma once

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <functional>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <commdlg.h>
#else
#  include <cstdio>
#endif

namespace Vortx
{
	class File 
	{
    public:
        static std::string SaveFileDialog(const char* defaultExt)
        {
        #ifdef _WIN32
            char filename[MAX_PATH] = {};
            OPENFILENAMEA ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.lpstrDefExt = defaultExt;
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
            if (GetSaveFileNameA(&ofn)) return std::string(filename);
            return {};
        #else
            char cmd[256];
            std::snprintf(cmd, sizeof(cmd),
                "zenity --file-selection --save --confirm-overwrite 2>/dev/null || "
                "kdialog --getsavefilename 2>/dev/null");
            FILE* pipe = popen(cmd, "r");
            if (!pipe) return {};
            char buf[4096] = {};
            if (fgets(buf, sizeof(buf), pipe)) {
                pclose(pipe);
                size_t len = strlen(buf);
                while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
                    buf[--len] = '\0';

                std::string result(buf);
                if (defaultExt && result.find('.') == std::string::npos)
                    result += std::string(".") + defaultExt;
                return result;
            }
            pclose(pipe);
            return {};
        #endif
        }

        static std::string OpenFileDialog()
        {
        #ifdef _WIN32
            char filename[MAX_PATH] = {};
            OPENFILENAMEA ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameA(&ofn)) return std::string(filename);
            return {};
        #else
            FILE* pipe = popen(
                "zenity --file-selection 2>/dev/null || "
                "kdialog --getopenfilename 2>/dev/null", "r");
            if (!pipe) return {};
            char buf[4096] = {};
            if (fgets(buf, sizeof(buf), pipe)) {
                pclose(pipe);
                size_t len = strlen(buf);
                while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
                    buf[--len] = '\0';
                return std::string(buf);
            }
            pclose(pipe);
            return {};
        #endif
        }
	};
}
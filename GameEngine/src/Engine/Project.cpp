#include "Project.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace Vortx
{
	Project Project::CreateProject(const std::string& path, const std::string& name)
	{
        std::filesystem::path projectPath = std::filesystem::path(path) / name;
        std::filesystem::path templatePath = std::filesystem::current_path() / "Assets" / "ProjectTemplate";

        if (std::filesystem::exists(projectPath))
        {
            std::cerr << "Project already exists" << std::endl;
            return Project();
        }

        std::filesystem::copy(templatePath, projectPath, std::filesystem::copy_options::recursive);

        std::ifstream in(projectPath / "CMakeLists.txt");
        std::string content((std::istreambuf_iterator<char>(in)), {});
        in.close();

        auto replace = [](std::string& str, const std::string& from, const std::string& to)
            {
                size_t pos = 0;
                while ((pos = str.find(from, pos)) != std::string::npos)
                {
                    str.replace(pos, from.size(), to);
                    pos += to.size();
                }
            };

        replace(content, "{{PROJECT_NAME}}", name);
        replace(content, "{{ENGINE_PATH}}", std::filesystem::current_path().string());

        std::ofstream out(projectPath / "CMakeLists.txt");
        out << content;

        return Project();
	}

	Project Project::LoadProject(const std::string& path)
	{
        return Project();
	}
}
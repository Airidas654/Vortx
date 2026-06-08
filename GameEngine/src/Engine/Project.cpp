#include "Project.h"

#include "Core/File.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace Vortx
{
    Project* Project::current = nullptr;


	Project Project::LoadProject(const std::string& path)
	{
        return Project();
	}
}
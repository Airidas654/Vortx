#pragma once

#include "AssetID.h"

#include "glm/glm.hpp"

#include <string>

namespace Vortx
{

	struct CameraData
	{
		float size = 5;
		glm::mat4 projMatrix = glm::mat4(1);
	};
}
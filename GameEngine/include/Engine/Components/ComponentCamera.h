#pragma once

#include "AssetID.h"

#include "glm/glm.hpp"

#include <string>

namespace Vortx
{

	struct CameraData
	{
		float size;
		glm::mat4 projMatrix;
	};
}
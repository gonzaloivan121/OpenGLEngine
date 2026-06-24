#pragma once

#include "ECS/Component.h"

#include <filesystem>

struct ShaderComponent : Component {
	std::filesystem::path ShaderFilepath;
};
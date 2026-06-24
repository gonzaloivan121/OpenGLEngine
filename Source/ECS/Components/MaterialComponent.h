#pragma once

#include "ECS/Component.h"
#include "ECS/Material.h"

#include <filesystem>

struct MaterialComponent : Component {
	Material Material;
	std::filesystem::path MaterialFilepath;
};
#pragma once

#include "ECS/Component.h"

#include <string>

struct NameComponent : Component {
	std::string Name = "Entity";
};
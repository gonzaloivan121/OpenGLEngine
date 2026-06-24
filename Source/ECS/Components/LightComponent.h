#pragma once

#include "ECS/Component.h"

#include <glm/glm.hpp>

enum class LightType { Directional, Point, Spot };

struct LightComponent : Component {
	LightType Type = LightType::Point;
	glm::vec3 Color = glm::vec3(1.0f);
	float Intensity = 1.0f;
	float Range = 10.0f;
	// Directional light specific
	glm::vec3 Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	// Point light specific
	float Constant = 1.0f;
	float Linear = 0.09f;
	float Quadratic = 0.032f;
	// Spot light specific
	float InnerCone = glm::cos(glm::radians(15.0f));
	float OuterCone = glm::cos(glm::radians(30.0f));
};
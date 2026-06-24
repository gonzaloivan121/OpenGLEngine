#pragma once

#include "ECS/Component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraComponent : Component {
	float FOV = 60.0f;
	float NearClip = 0.1f;
	float FarClip = 1000.0f;
	bool Primary = false;

	glm::mat4 GetProjection(float aspectRatio) const {
		return glm::perspective(glm::radians(FOV), aspectRatio, NearClip, FarClip);
	}
};
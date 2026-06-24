#pragma once

#include "ECS/Component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent : Component {
	glm::vec3 Position	= glm::vec3(0.0f);
	glm::vec3 Rotation	= glm::vec3(0.0f);
	glm::vec3 Scale		= glm::vec3(1.0f);

	glm::mat4 GetTransform() const {
		glm::mat4 t;

		t = glm::translate(glm::mat4(1.0f), Position);
		t = glm::rotate(t, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		t = glm::rotate(t, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		t = glm::rotate(t, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		t = glm::scale(t, Scale);

		return t;
	}
};
#include "SceneCamera.h"

#include "Core/Input/Input.h"
#include "Core/Input/Cursor.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

void SceneCamera::Update(Timestep ts, bool viewportHovered, bool viewportFocused) {
	const float deltaTime = ts.GetSeconds();
	const bool canNavigate = viewportHovered && viewportFocused && Input::IsMouseButtonPressed(MouseCode::ButtonRight);
	const bool navigationStartedThisFrame = canNavigate && !m_WasNavigatingLastFrame;

	const float yawRadians = glm::radians(m_Settings.Yaw);
	const float pitchRadians = glm::radians(m_Settings.Pitch);
	glm::vec3 forward(
		cosf(yawRadians) * cosf(pitchRadians),
		sinf(pitchRadians),
		sinf(yawRadians) * cosf(pitchRadians)
	);
	forward = glm::normalize(forward);

	const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	if (canNavigate) {
		Cursor::Lock();
		const glm::vec2 mouseDelta = navigationStartedThisFrame ? glm::vec2(0.0f) : Input::GetMouseDelta();
		m_Settings.Yaw += mouseDelta.x * m_Settings.RotationSpeed * deltaTime * 6.0f;
		m_Settings.Pitch -= mouseDelta.y * m_Settings.RotationSpeed * deltaTime * 6.0f;
		m_Settings.Pitch = std::clamp(m_Settings.Pitch, -89.0f, 89.0f);

		glm::vec3 moveDirection(0.0f);
		if (Input::IsKeyPressed(KeyCode::W)) moveDirection += forward;
		if (Input::IsKeyPressed(KeyCode::S)) moveDirection -= forward;
		if (Input::IsKeyPressed(KeyCode::A)) moveDirection -= right;
		if (Input::IsKeyPressed(KeyCode::D)) moveDirection += right;
		if (Input::IsKeyPressed(KeyCode::Q)) moveDirection -= up;
		if (Input::IsKeyPressed(KeyCode::E)) moveDirection += up;

		if (glm::length(moveDirection) > 0.0f) {
			moveDirection = glm::normalize(moveDirection);
			float movementSpeed = m_Settings.MovementSpeed;
			if (Input::IsKeyPressed(KeyCode::LeftShift)) {
				movementSpeed = m_Settings.FastMovementSpeed;
			}
			m_Settings.Position += moveDirection * movementSpeed * deltaTime;
		}
	} else {
		Cursor::Unlock();
	}

	if (viewportHovered) {
		const auto& scrollOffset = Input::GetScrollOffset();
		if (scrollOffset.y != 0.0f) {
			float zoomAmount = m_Settings.ZoomSpeed * static_cast<float>(scrollOffset.y);
			if (m_Settings.InvertZoom) {
				zoomAmount *= -1.0f;
			}
			m_Settings.Position += forward * zoomAmount;
		}
	}

	m_WasNavigatingLastFrame = canNavigate;
}

glm::mat4 SceneCamera::GetView() const {
	const float yawRadians = glm::radians(m_Settings.Yaw);
	const float pitchRadians = glm::radians(m_Settings.Pitch);
	glm::vec3 forward(
		cosf(yawRadians) * cosf(pitchRadians),
		sinf(pitchRadians),
		sinf(yawRadians) * cosf(pitchRadians)
	);
	forward = glm::normalize(forward);

	return glm::lookAt(m_Settings.Position, m_Settings.Position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 SceneCamera::GetProjection(float aspectRatio) const {
	return glm::perspective(glm::radians(m_Settings.FOV), aspectRatio, m_Settings.NearClip, m_Settings.FarClip);
}

glm::mat4 SceneCamera::GetViewProjection(float aspectRatio) const {
	return GetProjection(aspectRatio) * GetView();
}

glm::vec3 SceneCamera::GetPosition() const {
	return m_Settings.Position;
}
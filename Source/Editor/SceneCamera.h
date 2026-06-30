#pragma once

#include "Core/Timestep.h"
#include "Core/Settings/Settings.h"

#include <glm/glm.hpp>

class SceneCamera {
public:
	SceneCamera(SceneCameraSettings& settings) : m_Settings(settings) {}

	void Update(Timestep ts, bool viewportHovered, bool viewportFocused);
	glm::mat4 GetView() const;
	glm::mat4 GetProjection(float aspectRatio) const;
	glm::mat4 GetViewProjection(float aspectRatio) const;
	glm::vec3 GetPosition() const;
private:
	SceneCameraSettings& m_Settings;
	bool m_WasNavigatingLastFrame = false;
};
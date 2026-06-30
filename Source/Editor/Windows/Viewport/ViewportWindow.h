#pragma once

#include "Editor/Window.h"
#include "ECS/Scene.h"
#include "Renderer/Renderer.h"

#include <filesystem>
#include <functional>
#include <string>

#include <glm/glm.hpp>

class ViewportWindow : public Window {
public:
	enum class Mode {
		Scene,
		Game
	};

	using SceneLoadCallback = std::function<void(const std::filesystem::path&)>;

	ViewportWindow(bool& isOpen, Scene& scene, Mode mode = Mode::Scene)
		: Window(isOpen), m_Mode(mode), m_WindowTitle(mode == Mode::Scene ? "Scene View" : "Game"), m_Scene(scene) {}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;

	void SetSceneLoadCallback(const SceneLoadCallback& callback) { m_SceneLoadCallback = callback; }
private:
	Renderer::CameraData BuildSceneCamera(float aspectRatio) const;
	void EnsureFramebuffer();
	void Update();
	void Resize();
	void Draw();
private:
	Mode m_Mode = Mode::Scene;
	std::string m_WindowTitle = "Scene View";

	bool m_ViewportHovered = false;
	bool m_ViewportFocused = false;
	bool m_HasActivePrimaryCamera = true;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	Ref<Framebuffer> m_Framebuffer = nullptr;

	glm::vec3 m_SceneCameraPosition = glm::vec3(0.0f, 1.5f, 5.0f);
	float m_SceneCameraYaw = -90.0f;
	float m_SceneCameraPitch = -10.0f;
	float m_SceneCameraNearClip = 0.1f;
	float m_SceneCameraFarClip = 1000.0f;
	float m_SceneCameraFOV = 60.0f;

	Scene& m_Scene;

	SceneLoadCallback m_SceneLoadCallback;
};
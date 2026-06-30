#pragma once

#include "Editor/SceneCamera.h"
#include "Editor/Window.h"
#include "ECS/Scene.h"
#include "Renderer/Framebuffer.h"
#include "ImGuizmo.h"

#include <filesystem>
#include <functional>
#include <string>

class SceneViewWindow : public Window {
public:
	using SceneLoadCallback = std::function<void(const std::filesystem::path&)>;

	SceneViewWindow(bool& isOpen, Scene& scene, UUID& selectedEntityID);

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;

	void SetSceneLoadCallback(const SceneLoadCallback& callback) { m_SceneLoadCallback = callback; }
private:
	void Update();
	void Resize();
	void Draw();
	void DrawImage();
	void DrawGizmo();
	void EnsureFramebuffer();

	static Ref<Framebuffer> CreateFramebuffer(uint32_t width, uint32_t height);
private:
	Scene& m_Scene;
	UUID& m_SelectedEntityID;
	SceneCamera m_SceneCamera;

	SceneLoadCallback m_SceneLoadCallback;
	Ref<Framebuffer> m_Framebuffer = nullptr;

	bool m_ViewportHovered = false;
	bool m_ViewportFocused = false;
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;
};
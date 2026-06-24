#pragma once

#include "Editor/Window.h"
#include "ECS/Scene.h"

#include <filesystem>
#include <functional>

class ViewportWindow : public Window {
public:
	using SceneLoadCallback = std::function<void(const std::filesystem::path&)>;

	ViewportWindow(bool& isOpen, Scene& scene) : Window(isOpen), m_Scene(scene) {}

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
private:
	bool m_ViewportHovered = false;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	Scene& m_Scene;

	SceneLoadCallback m_SceneLoadCallback;
};
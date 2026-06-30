#pragma once

#include "Editor/Window.h"
#include "ECS/Scene.h"
#include "Renderer/Framebuffer.h"

#include <string>

class GameWindow : public Window {
public:
	GameWindow(bool& isOpen, Scene& scene);

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
private:
	void Update();
	void Resize();
	void Draw();
	void EnsureFramebuffer();

	static Ref<Framebuffer> CreateFramebuffer(uint32_t width, uint32_t height);
private:
	Scene& m_Scene;
	Ref<Framebuffer> m_Framebuffer = nullptr;

	bool m_WindowHovered = false;
	bool m_WindowFocused = false;
	bool m_HasActivePrimaryCamera = true;

	uint32_t m_WindowWidth = 0;
	uint32_t m_WindowHeight = 0;
};
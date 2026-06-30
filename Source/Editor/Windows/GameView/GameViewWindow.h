#pragma once

#include "Editor/Window.h"
#include "ECS/Scene.h"
#include "Renderer/Framebuffer.h"

#include <string>

class GameViewWindow : public Window {
public:
	GameViewWindow(bool& isOpen, Scene& scene);

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

	bool m_ViewportHovered = false;
	bool m_ViewportFocused = false;
	bool m_HasActivePrimaryCamera = true;
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	std::string m_WindowTitle = "Game";
};
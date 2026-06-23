#pragma once

#include "Core/Core.h"
#include "Core/Scene.h"

#include "Editor/Window.h"

#include <vector>

class InspectorWindow : public Window {
public:
	InspectorWindow(bool& isOpen, Scene& scene) : Window(isOpen), m_Scene(scene) {}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
private:
	Scene& m_Scene;
};
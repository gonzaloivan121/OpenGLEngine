#pragma once

#include "Core/Timestep.h"

#include "imgui.h"

class Window {
public:
	Window(bool& isOpen) : m_IsOpen(isOpen) {
		OnCreate();
	}

	virtual ~Window() = default;

	virtual void OnCreate() {}
	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(Timestep ts) {}
	virtual void OnUIRender() = 0;

	void Close() { m_IsOpen = false; }
protected:
	bool& m_IsOpen;
};
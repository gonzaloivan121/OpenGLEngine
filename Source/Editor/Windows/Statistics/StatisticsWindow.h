#pragma once

#include "Editor/Window.h"

class StatisticsWindow : public Window {
public:
	StatisticsWindow(bool& isOpen) : Window(isOpen) {}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
};
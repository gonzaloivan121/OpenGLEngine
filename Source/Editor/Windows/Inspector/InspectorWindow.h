#pragma once

#include "Core/Core.h"

#include "ECS/Components.h"
#include "ECS/Scene.h"

#include "Editor/Window.h"

#include <vector>

class InspectorWindow : public Window {
public:
	InspectorWindow(
		bool& isOpen,
		Scene& scene,
		UUID& selectedEntityID
	) :
		Window(isOpen),
		m_Scene(scene),
		m_SelectedEntityID(selectedEntityID)
	{}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
private:
	Scene& m_Scene;
	UUID& m_SelectedEntityID;

	std::vector<LightType> m_LightTypes;
	std::vector<AudioChannel> m_AudioChannels;
};
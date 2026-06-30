#pragma once

#include "Core/Core.h"
#include "ECS/Scene.h"

#include "Editor/Window.h"

#include <vector>

class HierarchyWindow : public Window {
public:
	HierarchyWindow(
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
	void CreateEntityPopup();
	void DeselectEntityIfClickedOutside();

	Entity CreateEmpty();
	Entity CreateCamera();
	Entity CreateDirectionalLight();
	Entity CreatePointLight();
	Entity CreateSpotLight();
private:
	Scene& m_Scene;
	UUID& m_SelectedEntityID;
};
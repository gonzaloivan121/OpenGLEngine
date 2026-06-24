#include "SceneWindow.h"

#include "Core/Log/Log.h"
#include "Core/Input/Input.h"

#include "ECS/Components.h"

#include "Editor/UI.h"

#include "Utilities/Utilities.h"

void SceneWindow::OnCreate() {
	Log::Trace("SceneWindow::OnCreate - Creating Inspector Window");
}

void SceneWindow::OnAttach() {
	Log::Trace("SceneWindow::OnAttach - Attaching Inspector Window");

	if (m_Scene.GetEntityIDs().empty()) {
		CreateEmpty();
	}
}

void SceneWindow::OnDetach() {
	Log::Trace("SceneWindow::OnDetach - Detaching Inspector Window");
}

void SceneWindow::OnUpdate(Timestep ts) {
	if (Input::IsKeyDown(KeyCode::Delete) && m_SelectedEntityID != 0) {
		m_Scene.DestroyEntity(m_SelectedEntityID);
		m_SelectedEntityID = UUID(0);
	}
}

void SceneWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	if (ImGui::Begin("Scene", &m_IsOpen)) {
		if (UI::Button("Create Entity")) {
			auto entity = m_Scene.CreateEntity("Entity");
		}

		UI::Separator();

		auto& entities = m_Scene.GetEntityIDs();
		UUID deleteCandidate = UUID(0);

		for (auto id : entities) {
			auto entity = m_Scene.GetEntity(id);
			auto* nameComponent = entity.GetComponent<NameComponent>();
			const std::string label = nameComponent != nullptr ? nameComponent->Name : "Entity";

			ImGui::PushID(static_cast<int>(id));

			if (ImGui::Selectable(label.c_str(), id == m_SelectedEntityID)) {
				m_SelectedEntityID = id;
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("Delete")) {
				deleteCandidate = id;
			}

			ImGui::PopID();
		}

		if (deleteCandidate != UUID(0)) {
			m_Scene.DestroyEntity(deleteCandidate);

			if (m_SelectedEntityID == deleteCandidate) {
				m_SelectedEntityID = UUID(0);
			}
		}
	}

	DeselectEntityIfClickedOutside();
	CreateEntityPopup();

	ImGui::End();
}

void SceneWindow::CreateEntityPopup() {
	if (ImGui::BeginPopupContextWindow("##CreateEntity",
		ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Empty")) {
				m_SelectedEntityID = CreateEmpty().ID;
			}

			if (ImGui::MenuItem("Camera")) {
				m_SelectedEntityID = CreateCamera().ID;
			}

			if (ImGui::BeginMenu("Light")) {
				if (ImGui::MenuItem("Directional")) {
					m_SelectedEntityID = CreateDirectionalLight().ID;
				}

				if (ImGui::MenuItem("Point")) {
					m_SelectedEntityID = CreatePointLight().ID;
				}

				if (ImGui::MenuItem("Spot")) {
					m_SelectedEntityID = CreateSpotLight().ID;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}

void SceneWindow::DeselectEntityIfClickedOutside() {
	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
		m_SelectedEntityID = UUID(0);
	}
}

Entity SceneWindow::CreateEmpty() {
	return m_Scene.CreateEntity("Empty");
}

Entity SceneWindow::CreateCamera() {
	auto entity = m_Scene.CreateEntity("Camera");
	entity.AddComponent<CameraComponent>();

	return entity;
}

Entity SceneWindow::CreateDirectionalLight() {
	auto entity = m_Scene.CreateEntity("Directional Light");
	auto& component = entity.AddComponent<LightComponent>();
	component.Type = LightType::Directional;

	return entity;
}

Entity SceneWindow::CreatePointLight() {
	auto entity = m_Scene.CreateEntity("Point Light");
	auto& component = entity.AddComponent<LightComponent>();
	component.Type = LightType::Point;

	return entity;
}

Entity SceneWindow::CreateSpotLight() {
	auto entity = m_Scene.CreateEntity("Spot Light");
	auto& component = entity.AddComponent<LightComponent>();
	component.Type = LightType::Spot;

	return entity;
}

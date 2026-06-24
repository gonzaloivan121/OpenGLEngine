#include "InspectorWindow.h"

#include "Core/Log/Log.h"

#include "ECS/ComponentRegistry.h"
#include "ECS/Serialization/Material/MaterialSerializer.h"

#include "Editor/UI.h"

#include "Utilities/Utilities.h"

#include <vector>

namespace {
	void RegisterComponentsIfNeeded() {
		static bool s_Registered = false;
		if (s_Registered) {
			return;
		}

		ComponentRegistry::Register<NameComponent>("Name");
		ComponentRegistry::Register<TransformComponent>("Transform");
		ComponentRegistry::Register<LightComponent>("Light");
		ComponentRegistry::Register<CameraComponent>("Camera");
		ComponentRegistry::Register<MaterialComponent>("Material");
		ComponentRegistry::Register<MeshComponent>("Mesh");
		ComponentRegistry::Register<ShaderComponent>("Shader");

		s_Registered = true;
	}
}

void InspectorWindow::OnCreate() {
	Log::Trace("InspectorWindow::OnCreate - Creating Inspector Window");
}

void InspectorWindow::OnAttach() {
	Log::Trace("InspectorWindow::OnAttach - Attaching Inspector Window");
	RegisterComponentsIfNeeded();

	m_LightTypes = { LightType::Directional, LightType::Point, LightType::Spot };
}

void InspectorWindow::OnDetach() {
	Log::Trace("InspectorWindow::OnDetach - Detaching Inspector Window");
}

void InspectorWindow::OnUpdate(Timestep ts) {}

void InspectorWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	if (ImGui::Begin("Inspector", &m_IsOpen)) {
		if (!m_Scene.IsValidEntity(m_SelectedEntityID)) {
			ImGui::Text("No entity selected.");
			ImGui::End();
			return;
		}

		auto entity = m_Scene.GetEntity(m_SelectedEntityID);

		if (auto* name = entity.GetComponent<NameComponent>()) {
			UI::InputText("Name", name->Name);
		}

		if (auto* transform = entity.GetComponent<TransformComponent>()) {
			if (UI::CollapsingHeader("Transform")) {
				UI::Vec3("Position", transform->Position);
				UI::Vec3("Rotation", transform->Rotation);
				UI::Vec3("Scale", transform->Scale, 1.0f);
			}
		}

		if (auto* light = entity.GetComponent<LightComponent>()) {
			if (UI::CollapsingHeader("Light")) {
 				UI::Dropdown("Type", m_LightTypes, light->Type, Utilities::LightTypeToString);
				UI::ColorEdit3("Color", light->Color);
				UI::DragFloat("Intensity", light->Intensity, 0.0f, 100.0f, 0.05f);
				UI::DragFloat("Range", light->Range, 0.1f, 1000.0f, 0.05f);

				if (light->Type == LightType::Directional || light->Type == LightType::Spot) {
					UI::Vec3("Direction", light->Direction);
				}

				if (light->Type == LightType::Point || light->Type == LightType::Spot) {
					UI::DragFloat("Constant", light->Constant, 0.0f, 10.0f, 0.001f);
					UI::DragFloat("Linear", light->Linear, 0.0f, 1.0f, 0.001f);
					UI::DragFloat("Quadratic", light->Quadratic, 0.0f, 1.0f, 0.001f);
				}

				if (light->Type == LightType::Spot) {
					UI::DragFloat("Inner Cone", light->InnerCone, 0.0f, 1.0f, 0.001f);
					UI::DragFloat("Outer Cone", light->OuterCone, 0.0f, 1.0f, 0.001f);
				}
			}
		}

		if (auto* camera = entity.GetComponent<CameraComponent>()) {
			if (UI::CollapsingHeader("Camera")) {
				UI::DragFloat("FOV", camera->FOV, 1.0f, 179.0f, 0.1f);
				UI::DragFloat("Near Clip", camera->NearClip, 0.001f, 100.0f, 0.001f);
				UI::DragFloat("Far Clip", camera->FarClip, 0.1f, 5000.0f, 1.0f);
				UI::Bool("Primary", camera->Primary);
			}
		}

		if (auto* materialComponent = entity.GetComponent<MaterialComponent>()) {
			if (UI::CollapsingHeader("Material")) {
				const bool changed = UI::MaterialSlot("Material", materialComponent->MaterialFilepath);

				if (changed && !materialComponent->MaterialFilepath.empty()) {
					MaterialSerializer serializer(materialComponent->Material);
					
					if (!serializer.Deserialize(materialComponent->MaterialFilepath)) {
						Log::Warning("InspectorWindow::OnUIRender - Couldn't deserialize Material: " + materialComponent->MaterialFilepath.string());
					}
				}

				if (!materialComponent->MaterialFilepath.empty()) {
					UI::Separator();

					auto& material = materialComponent->Material;
					bool materialAssetChanged = false;

					materialAssetChanged |= UI::ColorEdit4("Albedo", material.Albedo);
					materialAssetChanged |= UI::DragFloat("Metallic", material.Metallic, 0.0f, 1.0f, 0.01f);
					materialAssetChanged |= UI::DragFloat("Roughness", material.Roughness, 0.0f, 1.0f, 0.01f);
					materialAssetChanged |= UI::DragFloat("Ambient Occlusion", material.AmbientOcclusion, 0.0f, 1.0f, 0.01f);
					materialAssetChanged |= UI::DragFloat("Height Scale", material.HeightScale, 0.0f, 1.0f, 0.01f);

					if (UI::CollapsingHeader("Emission")) {
						auto& emission = material.Emission;

						materialAssetChanged |= UI::Bool("Enabled", emission.Enabled);

						if (emission.Enabled) {
							UI::Separator();

							materialAssetChanged |= UI::ColorEdit4("Color", emission.Color);
							materialAssetChanged |= UI::DragFloat("Intensity", emission.Intensity, 0.0f, 100.0f, 0.01f);
						}
					}

					if (UI::CollapsingHeader("Textures")) {
						auto& textures = material.Textures;

						materialAssetChanged |= UI::Vec2("Tiling", textures.Tiling, 0.01f);
						materialAssetChanged |= UI::Vec2("Offset", textures.Offset, 0.01f);

						UI::Separator();

						materialAssetChanged |= UI::TextureSlot("Albedo", textures.AlbedoFilepath);
						materialAssetChanged |= UI::TextureSlot("Normal", textures.NormalFilepath);
						materialAssetChanged |= UI::TextureSlot("Metallic", textures.MetallicFilepath);
						materialAssetChanged |= UI::TextureSlot("Roughness", textures.RoughnessFilepath);
						materialAssetChanged |= UI::TextureSlot("Ambient Occlusion", textures.AmbientOcclusionFilepath);
						materialAssetChanged |= UI::TextureSlot("Height", textures.HeightFilepath);
						materialAssetChanged |= UI::TextureSlot("Emission", textures.EmissionFilepath);
					}

					if (materialAssetChanged) {
						MaterialSerializer serializer(material);

						if (!serializer.Serialize(materialComponent->MaterialFilepath)) {
							Log::Warning("InspectorWindow::OnUIRender - Couldn't serialize Material: " + materialComponent->MaterialFilepath.string());
						}
					}
				}
			}
		}

		if (auto* meshComponent = entity.GetComponent<MeshComponent>()) {
			if (UI::CollapsingHeader("Mesh")) {
				UI::MeshSlot("Mesh", meshComponent->MeshFilepath);

				if (!meshComponent->MeshFilepath.empty()) {
					UI::Separator();

					UI::Bool("Cast Shadows", meshComponent->CastShadows);
					UI::Bool("Receive Shadows", meshComponent->ReceiveShadows);
				}
			}
		}

		if (auto* shaderComponent = entity.GetComponent<ShaderComponent>()) {
			if (UI::CollapsingHeader("Shader")) {
				UI::ShaderSlot("Shader", shaderComponent->ShaderFilepath);
			}
		}

		UI::Separator();
		ImGui::Text("Add Component");
		for (const auto& entry : ComponentRegistry::Entries()) {
			if (!entry.Has(entity)) {
				if (UI::Button(entry.Name.c_str())) {
					entry.Add(entity);
				}
			}
		}

		UI::Separator();
		ImGui::Text("Remove Component");
		for (const auto& entry : ComponentRegistry::Entries()) {
			if (entry.Has(entity) && entry.Name != "Name") {
				if (UI::Button(("Remove " + entry.Name).c_str())) {
					entry.Remove(entity);
					break;
				}
			}
		}
	}

	ImGui::End();
}
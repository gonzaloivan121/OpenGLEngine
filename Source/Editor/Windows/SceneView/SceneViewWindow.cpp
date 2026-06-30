#include "SceneViewWindow.h"

#include "Core/Settings/Manager/SettingsManager.h"
#include "Core/Log/Log.h"

#include "ECS/Components.h"

#include "Editor/Payload.h"
#include "Editor/UI.h"

#include "Renderer/Renderer.h"

#include "ImGuizmo.h"

#include <algorithm>
#include <cstring>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
	void ApplyTransformMatrix(TransformComponent& transform, const glm::mat4& matrix, ImGuizmo::OPERATION operation) {
		float translation[3];
		float rotation[3];
		float scale[3];
		float mutableMatrix[16];

		memcpy(mutableMatrix, glm::value_ptr(matrix), sizeof(mutableMatrix));
		ImGuizmo::DecomposeMatrixToComponents(mutableMatrix, translation, rotation, scale);

		switch (operation) {
			case ImGuizmo::TRANSLATE:
				transform.Position = { translation[0], translation[1], translation[2] };
				break;
			case ImGuizmo::ROTATE:
				transform.Rotation = { rotation[0], rotation[1], rotation[2] };
				break;
			case ImGuizmo::SCALE:
				transform.Scale = { scale[0], scale[1], scale[2] };
				break;
			default:
				transform.Position = { translation[0], translation[1], translation[2] };
				transform.Rotation = { rotation[0], rotation[1], rotation[2] };
				transform.Scale = { scale[0], scale[1], scale[2] };
				break;
		}
	}
}

SceneViewWindow::SceneViewWindow(bool& isOpen, Scene& scene, UUID& selectedEntityID)
	: Window(isOpen), m_Scene(scene), m_SelectedEntityID(selectedEntityID), m_SceneCamera(SettingsManager::Get().Editor.SceneCamera) {}

void SceneViewWindow::OnCreate() {
	Log::Trace("SceneViewWindow::OnCreate - Creating Scene View Window");
}

void SceneViewWindow::OnAttach() {
	Log::Trace("SceneViewWindow::OnAttach - Attaching Scene View Window");
	EnsureFramebuffer();
}

void SceneViewWindow::OnDetach() {
	Log::Trace("SceneViewWindow::OnDetach - Detaching Scene View Window");
	m_Framebuffer.reset();
}

void SceneViewWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || !m_Framebuffer || m_ViewportWidth == 0 || m_ViewportHeight == 0) {
		return;
	}

	m_SceneCamera.Update(ts, m_ViewportHovered, m_ViewportFocused);

	Renderer::RenderRequest request;
	request.TargetFramebuffer = m_Framebuffer;
	request.Source = Renderer::CameraSource::Override;
	request.OverrideCamera.Position = m_SceneCamera.GetPosition();
	request.OverrideCamera.View = m_SceneCamera.GetView();
	request.OverrideCamera.Projection = m_SceneCamera.GetProjection((float)m_ViewportWidth / (float)m_ViewportHeight);
	request.OverrideCamera.ViewProjection = request.OverrideCamera.Projection * request.OverrideCamera.View;

	Renderer::Begin();
	Renderer::Submit(m_Scene, request, nullptr);
	Renderer::End();
}

void SceneViewWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Scene View", &m_IsOpen)) {
		Update();
		Draw();
	}

	ImGui::End();
	ImGui::PopStyleVar();

	if (ImGui::Begin("Scene Controls", &m_IsOpen)) {
		if (UI::Button("Translate")) {
			m_GizmoOperation = ImGuizmo::TRANSLATE;
		}

		if (UI::Button("Rotate")) {
			m_GizmoOperation = ImGuizmo::ROTATE;
		}

		if (UI::Button("Scale")) {
			m_GizmoOperation = ImGuizmo::SCALE;
		}

		if (UI::Button("Local")) {
			m_GizmoMode = ImGuizmo::LOCAL;
		}

		if (UI::Button("World")) {
			m_GizmoMode = ImGuizmo::WORLD;
		}
	}
	ImGui::End();
}

void SceneViewWindow::Update() {
	const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	const auto& settings = SettingsManager::Get().Rendering.Resolution;

	m_ViewportWidth = static_cast<uint32_t>(std::max(0.0f, viewportSize.x * settings.Scale));
	m_ViewportHeight = static_cast<uint32_t>(std::max(0.0f, viewportSize.y * settings.Scale));

	EnsureFramebuffer();
	Resize();

	m_ViewportHovered = ImGui::IsWindowHovered();
	m_ViewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void SceneViewWindow::Resize() {
	if (m_Framebuffer && m_ViewportWidth > 0 && m_ViewportHeight > 0) {
		if (m_Framebuffer->GetWidth() != m_ViewportWidth || m_Framebuffer->GetHeight() != m_ViewportHeight) {
			m_Framebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
		}
	}
}

void SceneViewWindow::Draw() {
	if (!m_Framebuffer || m_ViewportWidth == 0 || m_ViewportHeight == 0) {
		return;
	}

	DrawImage();
	DrawGizmo();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Payload::ToUIPayload(Payload::Type::Scene))) {
			const char* path = (const char*)payload->Data;
			if (m_SceneLoadCallback) {
				m_SceneLoadCallback(std::filesystem::path(path));
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void SceneViewWindow::DrawImage() {
	if (const auto& finalImage = m_Framebuffer->GetColorAttachment()) {
		const auto& settings = SettingsManager::Get().Rendering.Resolution;

		ImGui::Image(
			(ImTextureRef)finalImage->GetHandle(),
			{(float)finalImage->GetWidth() / settings.Scale, (float)finalImage->GetHeight() / settings.Scale},
			{0, 1}, {1, 0}
		);
	}
}

void SceneViewWindow::DrawGizmo() {
	if (m_SelectedEntityID == UUID(0)) {
		return;
	}
	
	Entity entity = m_Scene.GetEntity(m_SelectedEntityID);
	auto* transformComponent = entity.GetComponent<TransformComponent>();
	if (!transformComponent) {
		return;
	}

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();

	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();

	if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
		return;
	}

	ImGuizmo::SetRect(windowPos.x, windowPos.y, windowWidth, windowHeight);

	const glm::mat4& cameraProjection = m_SceneCamera.GetProjection((float)m_ViewportWidth / (float)m_ViewportHeight);
	const glm::mat4& cameraView = m_SceneCamera.GetView();

	glm::mat4 transform = transformComponent->GetTransform();

	ImGuizmo::Manipulate(
		glm::value_ptr(cameraView),
		glm::value_ptr(cameraProjection),
		m_GizmoOperation,
		m_GizmoMode,
		glm::value_ptr(transform)
	);

	if (ImGuizmo::IsUsing()) {
		ApplyTransformMatrix(*transformComponent, transform, m_GizmoOperation);
	}
}

void SceneViewWindow::EnsureFramebuffer() {
	if (!m_Framebuffer) {
		const uint32_t width = std::max(1u, m_ViewportWidth);
		const uint32_t height = std::max(1u, m_ViewportHeight);
		m_Framebuffer = CreateFramebuffer(width, height);
	}
}

Ref<Framebuffer> SceneViewWindow::CreateFramebuffer(uint32_t width, uint32_t height) {
	TextureSpecification colorSpec;
	colorSpec.Width = width;
	colorSpec.Height = height;
	colorSpec.Format = TextureFormat::RGBA16F;
	colorSpec.MinFilter = TextureFilter::Linear;
	colorSpec.MagFilter = TextureFilter::Linear;
	colorSpec.WrapS = TextureWrap::ClampToEdge;
	colorSpec.WrapT = TextureWrap::ClampToEdge;
	colorSpec.GenerateMips = false;

	FramebufferSpecification specification;
	specification.Width = width;
	specification.Height = height;
	specification.ColorAttachments.push_back(colorSpec);
	specification.HasDepthAttachment = true;
	specification.DepthAttachmentSpecification.Format = TextureFormat::Depth24Stencil8;

	return Framebuffer::Create(specification);
}
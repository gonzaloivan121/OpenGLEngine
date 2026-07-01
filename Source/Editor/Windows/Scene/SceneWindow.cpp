#include "SceneWindow.h"

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

	bool DrawSegmentButton(const char* label, bool isActive, const glm::vec2& size = glm::vec2(0.0f, 0.0f)) {
		if (isActive) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Header));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}

		const bool clicked = UI::Button(label, size);

		if (isActive) {
			ImGui::PopStyleColor(3);
		}

		return clicked;
	}
}

SceneWindow::SceneWindow(bool& isOpen, Scene& scene, UUID& selectedEntityID)
	: Window(isOpen), m_Scene(scene), m_SelectedEntityID(selectedEntityID), m_SceneCamera(SettingsManager::Get().Editor.SceneCamera) {}

void SceneWindow::OnCreate() {
	Log::Trace("SceneWindow::OnCreate - Creating Scene View Window");
}

void SceneWindow::OnAttach() {
	Log::Trace("SceneWindow::OnAttach - Attaching Scene View Window");
	EnsureFramebuffer();
}

void SceneWindow::OnDetach() {
	Log::Trace("SceneWindow::OnDetach - Detaching Scene View Window");
	m_Framebuffer.reset();
}

void SceneWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || !m_Framebuffer || m_WindowWidth == 0 || m_WindowHeight == 0) {
		return;
	}

	m_SceneCamera.Update(ts, m_WindowHovered, m_WindowFocused);

	Renderer::RenderRequest request;
	request.TargetFramebuffer = m_Framebuffer;
	request.Source = Renderer::CameraSource::Override;
	request.OverrideCamera.Position = m_SceneCamera.GetPosition();
	request.OverrideCamera.View = m_SceneCamera.GetView();
	request.OverrideCamera.Projection = m_SceneCamera.GetProjection((float)m_WindowWidth / (float)m_WindowHeight);
	request.OverrideCamera.ViewProjection = request.OverrideCamera.Projection * request.OverrideCamera.View;

	Renderer::Begin();
	Renderer::Submit(m_Scene, request, nullptr);
	Renderer::End();
}

void SceneWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Scene", &m_IsOpen)) {
		Update();
		Draw();

		ImGui::SetCursorScreenPos({ m_SceneWindowPos.x + m_SceneWindowSize.x - 92.0f, m_SceneWindowPos.y + 8.0f });
		if (UI::Button("Controls", { 84.0f, 24.0f })) {
			m_ShowControlsPanel = !m_ShowControlsPanel;
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();

	if (m_ShowControlsPanel) {
		ImGui::SetNextWindowPos({ m_SceneWindowPos.x + m_SceneWindowSize.x + 12.0f, m_SceneWindowPos.y + 12.0f }, ImGuiCond_FirstUseEver);
		DrawSceneControlsPanel();
	}
}

void SceneWindow::Update() {
	const ImVec2 windowSize = ImGui::GetContentRegionAvail();
	const auto& settings = SettingsManager::Get().Rendering.Resolution;
	m_SceneWindowPos = ImGui::GetWindowPos();
	m_SceneWindowSize = ImGui::GetWindowSize();

	m_WindowWidth = static_cast<uint32_t>(std::max(0.0f, windowSize.x * settings.Scale));
	m_WindowHeight = static_cast<uint32_t>(std::max(0.0f, windowSize.y * settings.Scale));

	EnsureFramebuffer();
	Resize();

	m_WindowHovered = ImGui::IsWindowHovered();
	m_WindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void SceneWindow::Resize() {
	if (m_Framebuffer && m_WindowWidth > 0 && m_WindowHeight > 0) {
		if (m_Framebuffer->GetWidth() != m_WindowWidth || m_Framebuffer->GetHeight() != m_WindowHeight) {
			m_Framebuffer->Resize(m_WindowWidth, m_WindowHeight);
		}
	}
}

void SceneWindow::Draw() {
	if (!m_Framebuffer || m_WindowWidth == 0 || m_WindowHeight == 0) {
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

void SceneWindow::DrawImage() {
	if (const auto& finalImage = m_Framebuffer->GetColorAttachment()) {
		const auto& settings = SettingsManager::Get().Rendering.Resolution;

		ImGui::Image(
			(ImTextureRef)finalImage->GetHandle(),
			{(float)finalImage->GetWidth() / settings.Scale, (float)finalImage->GetHeight() / settings.Scale},
			{0, 1}, {1, 0}
		);
	}
}

void SceneWindow::DrawGizmo() {
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
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();

	if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
		return;
	}

	ImGuizmo::SetRect(windowPos.x, windowPos.y, windowWidth, windowHeight);

	const glm::mat4& cameraProjection = m_SceneCamera.GetProjection((float)m_WindowWidth / (float)m_WindowHeight);
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

void SceneWindow::DrawSceneControlsPanel() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

	const ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("Scene Controls", &m_ShowControlsPanel, panelFlags)) {
		ImGui::TextUnformatted("Transform");
		if (DrawSegmentButton("T", m_GizmoOperation == ImGuizmo::TRANSLATE, { 34.0f, 24.0f })) {
			m_GizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if (DrawSegmentButton("R", m_GizmoOperation == ImGuizmo::ROTATE, { 34.0f, 24.0f })) {
			m_GizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if (DrawSegmentButton("S", m_GizmoOperation == ImGuizmo::SCALE, { 34.0f, 24.0f })) {
			m_GizmoOperation = ImGuizmo::SCALE;
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::TextUnformatted("Space");
		if (DrawSegmentButton("Local", m_GizmoMode == ImGuizmo::LOCAL, { 58.0f, 24.0f })) {
			m_GizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::SameLine();
		if (DrawSegmentButton("World", m_GizmoMode == ImGuizmo::WORLD, { 58.0f, 24.0f })) {
			m_GizmoMode = ImGuizmo::WORLD;
		}
	}
	ImGui::End();

	ImGui::PopStyleVar(3);
}

void SceneWindow::EnsureFramebuffer() {
	if (!m_Framebuffer) {
		const uint32_t width = std::max(1u, m_WindowWidth);
		const uint32_t height = std::max(1u, m_WindowHeight);
		m_Framebuffer = CreateFramebuffer(width, height);
	}
}

Ref<Framebuffer> SceneWindow::CreateFramebuffer(uint32_t width, uint32_t height) {
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
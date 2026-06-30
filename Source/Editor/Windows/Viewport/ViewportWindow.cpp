#include "ViewportWindow.h"

#include "Core/Input/Input.h"
#include "Core/Settings/Manager/SettingsManager.h"

#include "Editor/Payload.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace {
	Ref<Framebuffer> CreateViewportFramebuffer(uint32_t width, uint32_t height) {
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
}

void ViewportWindow::OnCreate() {
	m_WindowTitle = (m_Mode == Mode::Scene) ? "Scene View" : "Game";
	Log::Trace("ViewportWindow::OnCreate - Creating " + m_WindowTitle + " Window");
}

void ViewportWindow::OnAttach() {
	Log::Trace("ViewportWindow::OnAttach - Attaching " + m_WindowTitle + " Window");
	EnsureFramebuffer();
}

void ViewportWindow::OnDetach() {
	Log::Trace("ViewportWindow::OnDetach - Detaching " + m_WindowTitle + " Window");
	m_Framebuffer.reset();
}

void ViewportWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || m_ViewportWidth == 0 || m_ViewportHeight == 0 || !m_Framebuffer) {
		return;
	}

	if (m_Mode == Mode::Scene) {
		const auto& settings = SettingsManager::Get().Navigation;
		const float deltaTime = ts.GetSeconds();
		const bool canNavigate = m_ViewportHovered && m_ViewportFocused && Input::IsMouseButtonPressed(MouseCode::ButtonRight);

		const float yawRadians = glm::radians(m_SceneCameraYaw);
		const float pitchRadians = glm::radians(m_SceneCameraPitch);
		glm::vec3 forward(
			cosf(yawRadians) * cosf(pitchRadians),
			sinf(pitchRadians),
			sinf(yawRadians) * cosf(pitchRadians)
		);
		forward = glm::normalize(forward);

		const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		if (canNavigate) {
			const glm::vec2 mouseDelta = Input::GetMouseDelta();
			m_SceneCameraYaw += mouseDelta.x * settings.RotationSpeed * deltaTime * 6.0f;
			m_SceneCameraPitch -= mouseDelta.y * settings.RotationSpeed * deltaTime * 6.0f;
			m_SceneCameraPitch = std::clamp(m_SceneCameraPitch, -89.0f, 89.0f);

			glm::vec3 moveDirection(0.0f);
			if (Input::IsKeyPressed(KeyCode::W)) moveDirection += forward;
			if (Input::IsKeyPressed(KeyCode::S)) moveDirection -= forward;
			if (Input::IsKeyPressed(KeyCode::A)) moveDirection -= right;
			if (Input::IsKeyPressed(KeyCode::D)) moveDirection += right;
			if (Input::IsKeyPressed(KeyCode::Q)) moveDirection -= up;
			if (Input::IsKeyPressed(KeyCode::E)) moveDirection += up;

			if (glm::length(moveDirection) > 0.0f) {
				moveDirection = glm::normalize(moveDirection);
				float movementSpeed = settings.MovementSpeed;
				if (Input::IsKeyPressed(KeyCode::LeftShift)) {
					movementSpeed = settings.FastMovementSpeed;
				}
				m_SceneCameraPosition += moveDirection * movementSpeed * deltaTime;
			}
		}

		if (m_ViewportHovered) {
			const auto& scrollOffset = Input::GetScrollOffset();
			if (scrollOffset.y != 0.0f) {
				float zoomAmount = settings.ZoomSpeed * static_cast<float>(scrollOffset.y);
				if (settings.InvertZoom) {
					zoomAmount *= -1.0f;
				}
				m_SceneCameraPosition += forward * zoomAmount;
			}
		}
	}

	Renderer::RenderRequest renderRequest;
	renderRequest.TargetFramebuffer = m_Framebuffer;

	if (m_Mode == Mode::Scene) {
		renderRequest.Source = Renderer::CameraSource::Override;
		renderRequest.OverrideCamera = BuildSceneCamera((float)m_ViewportWidth / (float)m_ViewportHeight);
	} else {
		renderRequest.Source = Renderer::CameraSource::PrimaryOnly;
	}

	Renderer::RenderResult result;
	Renderer::Begin();
	Renderer::Submit(m_Scene, renderRequest, &result);
	Renderer::End();

	if (m_Mode == Mode::Game) {
		m_HasActivePrimaryCamera = result.HasActivePrimaryCamera;
	}
}

void ViewportWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin(m_WindowTitle.c_str(), &m_IsOpen)) {
		Update();
		Draw();
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void ViewportWindow::Update() {
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	const auto& settings = SettingsManager::Get().Rendering.Resolution;

	m_ViewportWidth = static_cast<uint32_t>(std::max(0.0f, viewportSize.x * settings.Scale));
	m_ViewportHeight = static_cast<uint32_t>(std::max(0.0f, viewportSize.y * settings.Scale));

	EnsureFramebuffer();
	Resize();

	m_ViewportHovered = ImGui::IsWindowHovered();
	m_ViewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void ViewportWindow::Resize() {
	if (m_ViewportWidth > 0 && m_ViewportHeight > 0) {
		if (m_Framebuffer && (m_Framebuffer->GetWidth() != m_ViewportWidth || m_Framebuffer->GetHeight() != m_ViewportHeight)) {
			m_Framebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
		}
	}
}

void ViewportWindow::Draw() {
	if (!m_Framebuffer) {
		return;
	}

	Ref<Texture2D> finalImage = m_Framebuffer->GetColorAttachment();

	if (finalImage) {
		const auto& settings = SettingsManager::Get().Rendering.Resolution;

		ImGui::Image(
			(ImTextureRef)finalImage->GetHandle(),
			{ (float)finalImage->GetWidth() / settings.Scale, (float)finalImage->GetHeight() / settings.Scale },
			{ 0, 1 }, { 1, 0 }
		);
	}

	if (m_Mode == Mode::Game && !m_HasActivePrimaryCamera) {
		const char* fallbackText = "No active primary camera in the scene";
		const ImVec2 textSize = ImGui::CalcTextSize(fallbackText);
		const ImVec2 imageMin = ImGui::GetItemRectMin();
		const ImVec2 imageMax = ImGui::GetItemRectMax();
		const ImVec2 contentRegionSize(imageMax.x - imageMin.x, imageMax.y - imageMin.y);

		const ImVec2 textPosition(
			imageMin.x + (contentRegionSize.x - textSize.x) * 0.5f,
			imageMin.y + (contentRegionSize.y - textSize.y) * 0.5f
		);

		ImGui::GetWindowDrawList()->AddText(textPosition, IM_COL32(220, 220, 220, 255), fallbackText);
	}

	if (m_Mode == Mode::Scene && ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Payload::ToUIPayload(Payload::Type::Scene))) {
			const char* path = (const char*)payload->Data;

			if (m_SceneLoadCallback) {
				m_SceneLoadCallback(std::filesystem::path(path));
			}
		}

		ImGui::EndDragDropTarget();
	}
}

Renderer::CameraData ViewportWindow::BuildSceneCamera(float aspectRatio) const {
	Renderer::CameraData camera;

	const float yawRadians = glm::radians(m_SceneCameraYaw);
	const float pitchRadians = glm::radians(m_SceneCameraPitch);
	glm::vec3 forward(
		cosf(yawRadians) * cosf(pitchRadians),
		sinf(pitchRadians),
		sinf(yawRadians) * cosf(pitchRadians)
	);
	forward = glm::normalize(forward);

	camera.Position = m_SceneCameraPosition;
	camera.View = glm::lookAt(m_SceneCameraPosition, m_SceneCameraPosition + forward, glm::vec3(0.0f, 1.0f, 0.0f));
	camera.Projection = glm::perspective(glm::radians(m_SceneCameraFOV), aspectRatio, m_SceneCameraNearClip, m_SceneCameraFarClip);
	camera.ViewProjection = camera.Projection * camera.View;

	return camera;
}

void ViewportWindow::EnsureFramebuffer() {
	if (!m_Framebuffer) {
		const uint32_t width = std::max(1u, m_ViewportWidth);
		const uint32_t height = std::max(1u, m_ViewportHeight);
		m_Framebuffer = CreateViewportFramebuffer(width, height);
	}
}

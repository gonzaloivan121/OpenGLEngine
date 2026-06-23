#include "ViewportWindow.h"

#include "Core/Application/Application.h"
#include "Core/Input/Input.h"
#include "Core/Settings/SettingsManager.h"

#include "Renderer/Renderer.h"

#include "Editor/Payload.h"

void ViewportWindow::OnCreate() {
	Log::Trace("ViewportWindow::OnCreate - Creating Viewport Window");
}

void ViewportWindow::OnAttach() {
	Log::Trace("ViewportWindow::OnAttach - Attaching Viewport Window");
}

void ViewportWindow::OnDetach() {
	Log::Trace("ViewportWindow::OnDetach - Detaching Viewport Window");
}

void ViewportWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || m_ViewportWidth == 0 || m_ViewportHeight == 0) {
		return;
	}

	if (m_ViewportHovered) {
		const auto& settings = SettingsManager::Get().Navigation;

		glm::vec2 moveDirection = { 0.0f, 0.0f };

		if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) {
			moveDirection = Input::GetMouseDelta();
			moveDirection.x *= -1.0f;
		} else {
			if (Input::IsKeyPressed(KeyCode::W)) moveDirection.y += 1.0f;
			if (Input::IsKeyPressed(KeyCode::S)) moveDirection.y -= 1.0f;
			if (Input::IsKeyPressed(KeyCode::A)) moveDirection.x -= 1.0f;
			if (Input::IsKeyPressed(KeyCode::D)) moveDirection.x += 1.0f;
		}

		if (glm::length(moveDirection) > 0.0f) {
			moveDirection = glm::normalize(moveDirection);

			if (Input::IsKeyPressed(KeyCode::LeftShift)) {
				moveDirection *= settings.FastMovementSpeed * ts;
			}

			// TODO: Implement viewport movement
		}

		const auto& scrollOffset = Input::GetScrollOffset();

		if (scrollOffset.y != 0.0f) {
			double zoomFactor = 1.0f + settings.ZoomSpeed * ts * 10.0f;

			// Apply InvertZoom: positive scroll zooms in by default; negate when InvertZoom is true.
			bool zoomIn = settings.InvertZoom ? (scrollOffset.y < 0.0f) : (scrollOffset.y > 0.0f);

			// TODO: Implement zooming on the viewport.
			if (zoomIn) {

			} else {

			}
		}
	}

	Renderer::Begin();
	Renderer::Submit(m_Scene);
	Renderer::End();
}

void ViewportWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Viewport", &m_IsOpen)) {
		Update();
		Draw();
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void ViewportWindow::Update() {
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	const auto& settings = SettingsManager::Get().Rendering.Resolution;

	m_ViewportWidth = (uint32_t)viewportSize.x * settings.Scale;
	m_ViewportHeight = (uint32_t)viewportSize.y * settings.Scale;

	Resize();

	m_ViewportHovered = ImGui::IsWindowHovered();
}

void ViewportWindow::Resize() {
	if (m_ViewportWidth > 0 && m_ViewportHeight > 0) {
		// Resize the framebuffer
		if (auto framebuffer = Renderer::GetFramebuffer()) {
			if (auto colorAttachment = framebuffer->GetColorAttachment()) {
				if (colorAttachment->GetWidth() != m_ViewportWidth ||
					colorAttachment->GetHeight() != m_ViewportHeight
				) {
					framebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
				}
			}
		}
	}
}

void ViewportWindow::Draw() {
	Ref<Texture2D> finalImage = Renderer::GetFramebuffer()->GetColorAttachment();

	if (finalImage) {
		const auto& settings = SettingsManager::Get().Rendering.Resolution;

		ImGui::Image(
			(ImTextureRef)finalImage->GetHandle(),
			{ (float)finalImage->GetWidth() / settings.Scale, (float)finalImage->GetHeight() / settings.Scale },
			{ 0, 1 }, { 1, 0 }
		);
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Payload::Scene)) {
			const char* path = (const char*)payload->Data;

			if (m_SceneLoadCallback) {
				m_SceneLoadCallback(std::filesystem::path(path));
			}
		}

		ImGui::EndDragDropTarget();
	}
}

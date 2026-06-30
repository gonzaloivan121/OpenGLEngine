#include "GameWindow.h"

#include "Core/Settings/Manager/SettingsManager.h"
#include "Core/Log/Log.h"

#include "Editor/UI.h"

#include "Renderer/Renderer.h"

#include <algorithm>

GameWindow::GameWindow(bool& isOpen, Scene& scene)
	: Window(isOpen), m_Scene(scene) {}

void GameWindow::OnCreate() {
	Log::Trace("GameWindow::OnCreate - Creating Game Window");
}

void GameWindow::OnAttach() {
	Log::Trace("GameWindow::OnAttach - Attaching Game Window");
	EnsureFramebuffer();
}

void GameWindow::OnDetach() {
	Log::Trace("GameWindow::OnDetach - Detaching Game Window");
	m_Framebuffer.reset();
}

void GameWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || !m_Framebuffer) {
		return;
	}

	Renderer::RenderRequest request;
	request.TargetFramebuffer = m_Framebuffer;
	request.Source = Renderer::CameraSource::PrimaryOnly;

	Renderer::RenderResult result;
	Renderer::Begin();
	Renderer::Submit(m_Scene, request, &result);
	Renderer::End();

	m_HasActivePrimaryCamera = result.HasActivePrimaryCamera;
}

void GameWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Game", &m_IsOpen)) {
		Update();
		Draw();
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void GameWindow::Update() {
	const ImVec2 windowSize = ImGui::GetContentRegionAvail();
	const auto& settings = SettingsManager::Get().Rendering.Resolution;

	m_WindowWidth = static_cast<uint32_t>(std::max(0.0f, windowSize.x * settings.Scale));
	m_WindowHeight = static_cast<uint32_t>(std::max(0.0f, windowSize.y * settings.Scale));

	EnsureFramebuffer();
	Resize();

	m_WindowHovered = ImGui::IsWindowHovered();
	m_WindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void GameWindow::Resize() {
	if (m_Framebuffer && m_WindowWidth > 0 && m_WindowHeight > 0) {
		if (m_Framebuffer->GetWidth() != m_WindowWidth || m_Framebuffer->GetHeight() != m_WindowHeight) {
			m_Framebuffer->Resize(m_WindowWidth, m_WindowHeight);
		}
	}
}

void GameWindow::Draw() {
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

	if (!m_HasActivePrimaryCamera) {
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
}

void GameWindow::EnsureFramebuffer() {
	if (!m_Framebuffer) {
		const uint32_t width = std::max(1u, m_WindowWidth);
		const uint32_t height = std::max(1u, m_WindowHeight);
		m_Framebuffer = CreateFramebuffer(width, height);
	}
}

Ref<Framebuffer> GameWindow::CreateFramebuffer(uint32_t width, uint32_t height) {
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
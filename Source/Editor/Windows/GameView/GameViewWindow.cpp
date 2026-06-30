#include "GameViewWindow.h"

#include "Core/Settings/Manager/SettingsManager.h"
#include "Core/Log/Log.h"

#include "Editor/UI.h"

#include "Renderer/Renderer.h"

#include <algorithm>

GameViewWindow::GameViewWindow(bool& isOpen, Scene& scene)
	: Window(isOpen), m_Scene(scene) {}

void GameViewWindow::OnCreate() {
	Log::Trace("GameViewWindow::OnCreate - Creating Game Window");
}

void GameViewWindow::OnAttach() {
	Log::Trace("GameViewWindow::OnAttach - Attaching Game Window");
	EnsureFramebuffer();
}

void GameViewWindow::OnDetach() {
	Log::Trace("GameViewWindow::OnDetach - Detaching Game Window");
	m_Framebuffer.reset();
}

void GameViewWindow::OnUpdate(Timestep ts) {
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

void GameViewWindow::OnUIRender() {
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

void GameViewWindow::Update() {
	const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	const auto& settings = SettingsManager::Get().Rendering.Resolution;

	m_ViewportWidth = static_cast<uint32_t>(std::max(0.0f, viewportSize.x * settings.Scale));
	m_ViewportHeight = static_cast<uint32_t>(std::max(0.0f, viewportSize.y * settings.Scale));

	EnsureFramebuffer();
	Resize();

	m_ViewportHovered = ImGui::IsWindowHovered();
	m_ViewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void GameViewWindow::Resize() {
	if (m_Framebuffer && m_ViewportWidth > 0 && m_ViewportHeight > 0) {
		if (m_Framebuffer->GetWidth() != m_ViewportWidth || m_Framebuffer->GetHeight() != m_ViewportHeight) {
			m_Framebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
		}
	}
}

void GameViewWindow::Draw() {
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

void GameViewWindow::EnsureFramebuffer() {
	if (!m_Framebuffer) {
		const uint32_t width = std::max(1u, m_ViewportWidth);
		const uint32_t height = std::max(1u, m_ViewportHeight);
		m_Framebuffer = CreateFramebuffer(width, height);
	}
}

Ref<Framebuffer> GameViewWindow::CreateFramebuffer(uint32_t width, uint32_t height) {
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
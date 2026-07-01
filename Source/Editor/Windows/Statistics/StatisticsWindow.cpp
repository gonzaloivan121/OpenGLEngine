#include "StatisticsWindow.h"

#include "Core/Application/Application.h"
#include "Core/Audio/AudioEngine.h"
#include "Core/Input/Input.h"
#include "Core/Log/Log.h"
#include "Core/Settings/Manager/SettingsManager.h"

#include "ECS/Components.h"

#include "Editor/UI.h"

#include <algorithm>
#include <cstddef>

namespace {
	constexpr float MinSamplingIntervalMs = 16.0f;
	constexpr float MaxSamplingIntervalMs = 1000.0f;
	constexpr int MinHistorySize = 32;
	constexpr int MaxHistorySize = 1200;

	const char* PanelName(StatisticsWindow::Panel panel) {
		switch (panel) {
			case StatisticsWindow::Panel::Performance: 	return "Performance";
			case StatisticsWindow::Panel::Renderer: 	return "Renderer";
			case StatisticsWindow::Panel::Scene: 		return "Scene / ECS";
			case StatisticsWindow::Panel::Lighting: 	return "Lighting";
			case StatisticsWindow::Panel::Memory: 		return "Memory";
			case StatisticsWindow::Panel::Audio: 		return "Audio";
			case StatisticsWindow::Panel::Input: 		return "Input";
			case StatisticsWindow::Panel::System: 		return "System";
			default: 									return "Unknown";
		}
	}
}

StatisticsWindow::StatisticsWindow(bool& isOpen, Scene& scene)
	: Window(isOpen), m_Scene(scene) {}

void StatisticsWindow::OnCreate() {
	Log::Trace("StatisticsWindow::OnCreate - Creating Statistics Window");
}

void StatisticsWindow::OnAttach() {
	Log::Trace("StatisticsWindow::OnAttach - Attaching Statistics Window");

	auto& statsSettings = SettingsManager::Get().Editor.Statistics;
	statsSettings.SamplingIntervalMs = std::clamp(statsSettings.SamplingIntervalMs, MinSamplingIntervalMs, MaxSamplingIntervalMs);
	statsSettings.HistorySize = std::clamp(statsSettings.HistorySize, MinHistorySize, MaxHistorySize);
	m_ActivePanel = static_cast<Panel>(std::clamp(statsSettings.ActivePanel, 0, static_cast<int>(Panel::System)));
	m_Paused = statsSettings.StartPaused;

	CollectSnapshot();
	PushHistorySample(m_Snapshot.Application.FrameTimeMs, m_Snapshot.Application.FPS);
}

void StatisticsWindow::OnDetach() {
	Log::Trace("StatisticsWindow::OnDetach - Detaching Statistics Window");
}

void StatisticsWindow::OnUpdate(Timestep ts) {
	if (!m_IsOpen || m_Paused) {
		return;
	}

	const auto& statisticsSettings = SettingsManager::Get().Editor.Statistics;
	m_SampleTimerMs += ts.GetMilliseconds();

	if (m_SampleTimerMs >= statisticsSettings.SamplingIntervalMs) {
		m_SampleTimerMs = 0.0f;
		CollectSnapshot();
		PushHistorySample(m_Snapshot.Application.FrameTimeMs, m_Snapshot.Application.FPS);
	}
}

void StatisticsWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	bool settingsDirty = false;

	ImGui::Begin("Statistics", &m_IsOpen, ImGuiWindowFlags_MenuBar);

	DrawControls(settingsDirty);
	DrawDashboard();

	ImGui::Separator();
	ImGui::Columns(2, nullptr, true);
	ImGui::SetColumnWidth(0, 220.0f);

	DrawNavigation(settingsDirty);

	ImGui::NextColumn();
	DrawDetailPanel();

	ImGui::Columns(1);

	if (settingsDirty) {
		auto& statsSettings = SettingsManager::Get().Editor.Statistics;
		statsSettings.ActivePanel = static_cast<int>(m_ActivePanel);
		statsSettings.StartPaused = m_Paused;
		SettingsManager::Save();
	}

	ImGui::End();
}

void StatisticsWindow::CollectSnapshot() {
	m_Snapshot.Application = Application::Get().GetRuntimeStats();
	m_Snapshot.Renderer = Renderer::GetStats();
	m_Snapshot.Scene = CollectSceneStats();
}

StatisticsWindow::SceneStats StatisticsWindow::CollectSceneStats() const {
	SceneStats stats;
	const auto& ids = m_Scene.GetEntityIDs();
	stats.EntityCount = static_cast<uint32_t>(ids.size());

	for (const auto& id : ids) {
		Entity entity = m_Scene.GetEntity(id);

		if (entity.HasComponent<TransformComponent>()) {
			stats.TransformCount++;
		}

		if (const auto* camera = entity.GetComponent<CameraComponent>()) {
			stats.CameraCount++;
			if (camera->Primary) {
				stats.PrimaryCameraCount++;
			}
		}

		if (auto* mesh = entity.GetComponent<MeshComponent>()) {
			stats.MeshCount++;
			if (mesh->Mesh) {
				stats.MeshVertices += mesh->Mesh->GetVertexCount();
				stats.MeshTriangles += mesh->Mesh->GetTriangleCount();
			}
		}

		if (entity.HasComponent<MaterialComponent>()) {
			stats.MaterialCount++;
		}

		if (const auto* light = entity.GetComponent<LightComponent>()) {
			stats.LightCount++;
			switch (light->Type) {
				case LightType::Directional: 	stats.DirectionalLights++; 	break;
				case LightType::Point: 			stats.PointLights++; 		break;
				case LightType::Spot: 			stats.SpotLights++; 		break;
			}
		}

		if (entity.HasComponent<AudioComponent>()) {
			stats.AudioCount++;
		}
	}

	return stats;
}

void StatisticsWindow::PushHistorySample(float frameTimeMs, float fps) {
	m_FrameTimeHistory.push_back(frameTimeMs);
	m_FPSHistory.push_back(fps);
	ApplyHistorySizeLimit();
}

void StatisticsWindow::ApplyHistorySizeLimit() {
	const int historySize = std::clamp(SettingsManager::Get().Editor.Statistics.HistorySize, MinHistorySize, MaxHistorySize);

	if (static_cast<int>(m_FrameTimeHistory.size()) > historySize) {
		const size_t eraseCount = m_FrameTimeHistory.size() - static_cast<size_t>(historySize);
		m_FrameTimeHistory.erase(m_FrameTimeHistory.begin(), m_FrameTimeHistory.begin() + static_cast<std::ptrdiff_t>(eraseCount));
	}

	if (static_cast<int>(m_FPSHistory.size()) > historySize) {
		const size_t eraseCount = m_FPSHistory.size() - static_cast<size_t>(historySize);
		m_FPSHistory.erase(m_FPSHistory.begin(), m_FPSHistory.begin() + static_cast<std::ptrdiff_t>(eraseCount));
	}
}

void StatisticsWindow::EnsureActivePanelVisible() {
	const auto& settings = SettingsManager::Get().Editor.Statistics;

	auto visible = [&settings](Panel panel) {
		switch (panel) {
			case Panel::Performance: 	return settings.ShowPerformance;
			case Panel::Renderer: 		return settings.ShowRenderer;
			case Panel::Scene: 			return settings.ShowScene;
			case Panel::Lighting: 		return settings.ShowLighting;
			case Panel::Memory: 		return settings.ShowMemory;
			case Panel::Audio: 			return settings.ShowAudio;
			case Panel::Input: 			return settings.ShowInput;
			case Panel::System: 		return settings.ShowSystem;
			default: 					return true;
		}
	};

	if (visible(m_ActivePanel)) {
		return;
	}

	for (int i = 0; i <= static_cast<int>(Panel::System); ++i) {
		Panel candidate = static_cast<Panel>(i);
		if (visible(candidate)) {
			m_ActivePanel = candidate;
			return;
		}
	}
}

void StatisticsWindow::DrawControls(bool& settingsDirty) {
	auto& settings = SettingsManager::Get().Editor.Statistics;

	if (UI::Button(m_Paused ? "Resume" : "Pause")) {
		m_Paused = !m_Paused;
		settingsDirty = true;
	}
	ImGui::SameLine();
	if (UI::Button("Refresh")) {
		CollectSnapshot();
		PushHistorySample(m_Snapshot.Application.FrameTimeMs, m_Snapshot.Application.FPS);
	}

	ImGui::SameLine();
	ImGui::TextDisabled("Interval (ms)");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110.0f);
	if (ImGui::DragFloat("##stats_interval", &settings.SamplingIntervalMs, 1.0f, MinSamplingIntervalMs, MaxSamplingIntervalMs, "%.0f")) {
		settings.SamplingIntervalMs = std::clamp(settings.SamplingIntervalMs, MinSamplingIntervalMs, MaxSamplingIntervalMs);
		settingsDirty = true;
	}

	ImGui::SameLine();
	ImGui::TextDisabled("History");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragInt("##stats_history", &settings.HistorySize, 1.0f, MinHistorySize, MaxHistorySize)) {
		settings.HistorySize = std::clamp(settings.HistorySize, MinHistorySize, MaxHistorySize);
		ApplyHistorySizeLimit();
		settingsDirty = true;
	}

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Sections")) {
			settingsDirty |= ImGui::MenuItem("Performance", nullptr, &settings.ShowPerformance);
			settingsDirty |= ImGui::MenuItem("Renderer", nullptr, &settings.ShowRenderer);
			settingsDirty |= ImGui::MenuItem("Scene / ECS", nullptr, &settings.ShowScene);
			settingsDirty |= ImGui::MenuItem("Lighting", nullptr, &settings.ShowLighting);
			settingsDirty |= ImGui::MenuItem("Memory", nullptr, &settings.ShowMemory);
			settingsDirty |= ImGui::MenuItem("Audio", nullptr, &settings.ShowAudio);
			settingsDirty |= ImGui::MenuItem("Input", nullptr, &settings.ShowInput);
			settingsDirty |= ImGui::MenuItem("System", nullptr, &settings.ShowSystem);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	EnsureActivePanelVisible();
}

void StatisticsWindow::DrawDashboard() const {
	if (ImGui::BeginTable("StatisticsKPIs", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Frame: %.2f ms", m_Snapshot.Application.SmoothedFrameTimeMs);
		ImGui::Text("FPS: %.1f", m_Snapshot.Application.SmoothedFPS);

		ImGui::TableSetColumnIndex(1);
		ImGui::Text("Draw Calls: %u", m_Snapshot.Renderer.DrawCalls);
		ImGui::Text("Triangles: %llu", static_cast<unsigned long long>(m_Snapshot.Renderer.TriangleCount));

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("Entities: %u", m_Snapshot.Scene.EntityCount);
		ImGui::Text("Lights: %u", m_Snapshot.Renderer.TotalLights);

		ImGui::TableSetColumnIndex(3);
		const float outputBytes = static_cast<float>(m_Snapshot.Renderer.OutputWidth) * static_cast<float>(m_Snapshot.Renderer.OutputHeight) * 12.0f;
		const float gBufferBytes = static_cast<float>(m_Snapshot.Renderer.GBufferWidth) * static_cast<float>(m_Snapshot.Renderer.GBufferHeight) * 28.0f;
		const float totalMB = (outputBytes + gBufferBytes) / (1024.0f * 1024.0f);
		ImGui::Text("FB Mem*: %.2f MB", totalMB);
		ImGui::Text("GPU Timers: %s", m_Snapshot.Renderer.GPUTimingAvailable ? "Available" : "Unavailable");

		ImGui::EndTable();
	}

	ImGui::TextDisabled("*Estimated framebuffer memory only");
}

void StatisticsWindow::DrawNavigation(bool& settingsDirty) {
	const auto& settings = SettingsManager::Get().Editor.Statistics;

	auto drawPanelEntry = [this, &settingsDirty](Panel panel, bool visible) {
		if (!visible) {
			return;
		}

		const bool selected = m_ActivePanel == panel;
		if (ImGui::Selectable(PanelName(panel), selected)) {
			m_ActivePanel = panel;
			settingsDirty = true;
		}
	};

	drawPanelEntry(Panel::Performance, settings.ShowPerformance);
	drawPanelEntry(Panel::Renderer, settings.ShowRenderer);
	drawPanelEntry(Panel::Scene, settings.ShowScene);
	drawPanelEntry(Panel::Lighting, settings.ShowLighting);
	drawPanelEntry(Panel::Memory, settings.ShowMemory);
	drawPanelEntry(Panel::Audio, settings.ShowAudio);
	drawPanelEntry(Panel::Input, settings.ShowInput);
	drawPanelEntry(Panel::System, settings.ShowSystem);
}

void StatisticsWindow::DrawDetailPanel() const {
	switch (m_ActivePanel) {
		case Panel::Performance:	DrawPerformancePanel();		break;
		case Panel::Renderer:		DrawRendererPanel();		break;
		case Panel::Scene:			DrawScenePanel();			break;
		case Panel::Lighting:		DrawLightingPanel();		break;
		case Panel::Memory:			DrawMemoryPanel();			break;
		case Panel::Audio:			DrawAudioPanel();			break;
		case Panel::Input:			DrawInputPanel();			break;
		case Panel::System:			DrawSystemPanel();			break;
		default:												break;
	}
}

void StatisticsWindow::DrawPerformancePanel() const {
	std::string instantFrameStr = std::to_string(m_Snapshot.Application.FrameTimeMs) + " ms";
	std::string smoothedFrameStr = std::to_string(m_Snapshot.Application.SmoothedFrameTimeMs) + " ms";
	std::string instantFpsStr = std::to_string(m_Snapshot.Application.FPS) + " FPS";
	std::string smoothedFpsStr = std::to_string(m_Snapshot.Application.SmoothedFPS) + " FPS";

	UI::DisabledInputText("Instant Frame Time", instantFrameStr);
	UI::DisabledInputText("Smoothed Frame Time", smoothedFrameStr);
	UI::DisabledInputText("Instant FPS", instantFpsStr);
	UI::DisabledInputText("Smoothed FPS", smoothedFpsStr);

	if (m_Snapshot.Application.FrameBudgetMs > 0.0f) {
		std::string frameBudgetStr = std::to_string(m_Snapshot.Application.FrameBudgetMs) + " ms";
		std::string budgetUsageStr = std::to_string(m_Snapshot.Application.FrameBudgetUsage) + " %";

		UI::DisabledInputText("Frame Budget", frameBudgetStr);
		UI::DisabledInputText("Budget Usage", budgetUsageStr);
	}

	if (!m_FrameTimeHistory.empty()) {
		ImGui::Separator();
		ImGui::PlotLines("Frame Time (ms)", m_FrameTimeHistory.data(), static_cast<int>(m_FrameTimeHistory.size()), 0, nullptr, 0.0f, 50.0f, ImVec2(0.0f, 90.0f));
	}

	if (!m_FPSHistory.empty()) {
		ImGui::PlotLines("FPS", m_FPSHistory.data(), static_cast<int>(m_FPSHistory.size()), 0, nullptr, 0.0f, 240.0f, ImVec2(0.0f, 90.0f));
	}
}

void StatisticsWindow::DrawRendererPanel() const {
	ImGui::Text("Rendered: %s", m_Snapshot.Renderer.Rendered ? "Yes" : "No");
	ImGui::Text("Primary Camera: %s", m_Snapshot.Renderer.HasActivePrimaryCamera ? "Available" : "Missing");
	ImGui::Text("Camera Source: %s", m_Snapshot.Renderer.UsedOverrideCamera ? "Override" : "Scene Camera");
	ImGui::Separator();

	ImGui::Text("Draw Calls: %u", m_Snapshot.Renderer.DrawCalls);
	ImGui::Text("Mesh Draw Calls: %u", m_Snapshot.Renderer.MeshDrawCalls);
	ImGui::Text("Fullscreen Draw Calls: %u", m_Snapshot.Renderer.FullscreenDrawCalls);
	ImGui::Text("Rendered Entities: %u", m_Snapshot.Renderer.RenderedEntities);
	ImGui::Text("Material-bound Entities: %u", m_Snapshot.Renderer.MaterialBoundEntities);
	ImGui::Text("Vertices: %llu", static_cast<unsigned long long>(m_Snapshot.Renderer.VertexCount));
	ImGui::Text("Triangles: %llu", static_cast<unsigned long long>(m_Snapshot.Renderer.TriangleCount));
	ImGui::Text("Indices: %llu", static_cast<unsigned long long>(m_Snapshot.Renderer.IndexCount));
	ImGui::Separator();

	ImGui::Text("Output: %ux%u", m_Snapshot.Renderer.OutputWidth, m_Snapshot.Renderer.OutputHeight);
	ImGui::Text("GBuffer: %ux%u", m_Snapshot.Renderer.GBufferWidth, m_Snapshot.Renderer.GBufferHeight);
	ImGui::Text("Shader Cache: %zu", m_Snapshot.Renderer.ShaderCacheSize);
	ImGui::Separator();

	ImGui::Text("Geometry CPU: %.3f ms", m_Snapshot.Renderer.GeometryPassCPUTimeMs);
	ImGui::Text("Background CPU: %.3f ms", m_Snapshot.Renderer.BackgroundPassCPUTimeMs);
	ImGui::Text("Lighting CPU: %.3f ms", m_Snapshot.Renderer.LightingPassCPUTimeMs);
	ImGui::Text("Total Submit CPU: %.3f ms", m_Snapshot.Renderer.TotalCPUTimeMs);
}

void StatisticsWindow::DrawScenePanel() const {
	ImGui::Text("Entities: %u", m_Snapshot.Scene.EntityCount);
	ImGui::Text("Transforms: %u", m_Snapshot.Scene.TransformCount);
	ImGui::Text("Cameras: %u", m_Snapshot.Scene.CameraCount);
	ImGui::Text("Primary Cameras: %u", m_Snapshot.Scene.PrimaryCameraCount);
	ImGui::Text("Meshes: %u", m_Snapshot.Scene.MeshCount);
	ImGui::Text("Materials: %u", m_Snapshot.Scene.MaterialCount);
	ImGui::Text("Audio Components: %u", m_Snapshot.Scene.AudioCount);
	ImGui::Separator();
	ImGui::Text("Scene Mesh Vertices: %llu", static_cast<unsigned long long>(m_Snapshot.Scene.MeshVertices));
	ImGui::Text("Scene Mesh Triangles: %llu", static_cast<unsigned long long>(m_Snapshot.Scene.MeshTriangles));
}

void StatisticsWindow::DrawLightingPanel() const {
	ImGui::Text("Total Lights: %u", m_Snapshot.Renderer.TotalLights);
	ImGui::Text("Directional: %u", m_Snapshot.Renderer.DirectionalLights);
	ImGui::Text("Point: %u", m_Snapshot.Renderer.PointLights);
	ImGui::Text("Spot: %u", m_Snapshot.Renderer.SpotLights);
	ImGui::Text("Per-type Cap: %u", m_Snapshot.Renderer.MaxLightsPerType);
	ImGui::Separator();
	ImGui::Text("Scene Directional: %u", m_Snapshot.Scene.DirectionalLights);
	ImGui::Text("Scene Point: %u", m_Snapshot.Scene.PointLights);
	ImGui::Text("Scene Spot: %u", m_Snapshot.Scene.SpotLights);
}

void StatisticsWindow::DrawMemoryPanel() const {
	const float outputBytes = static_cast<float>(m_Snapshot.Renderer.OutputWidth) * static_cast<float>(m_Snapshot.Renderer.OutputHeight) * 12.0f;
	const float gBufferBytes = static_cast<float>(m_Snapshot.Renderer.GBufferWidth) * static_cast<float>(m_Snapshot.Renderer.GBufferHeight) * 28.0f;
	const float totalBytes = outputBytes + gBufferBytes;

	ImGui::Text("Estimated Output FB: %.2f MB", outputBytes / (1024.0f * 1024.0f));
	ImGui::Text("Estimated GBuffer: %.2f MB", gBufferBytes / (1024.0f * 1024.0f));
	ImGui::Text("Estimated Total: %.2f MB", totalBytes / (1024.0f * 1024.0f));
	ImGui::Separator();
	ImGui::Text("Shader Cache Entries: %zu", m_Snapshot.Renderer.ShaderCacheSize);
	ImGui::TextDisabled("Memory values are conservative estimates based on attachment formats.");
}

void StatisticsWindow::DrawAudioPanel() const {
	const auto& inputs = AudioEngine::GetInputDevices();
	const auto& outputs = AudioEngine::GetOutputDevices();

	ImGui::Text("Input Devices: %zu", inputs.size());
	ImGui::Text("Output Devices: %zu", outputs.size());
	ImGui::Text("Default Input Device ID: %d", AudioEngine::GetDefaultInputDeviceID());
	ImGui::Text("Default Output Device ID: %d", AudioEngine::GetDefaultOutputDeviceID());
	ImGui::Separator();
	ImGui::Text("Entities With Audio Components: %u", m_Snapshot.Scene.AudioCount);
	ImGui::TextDisabled("Active clip/channel telemetry will be expanded in next instrumentation pass.");
}

void StatisticsWindow::DrawInputPanel() const {
	const ImGuiIO& io = ImGui::GetIO();
	const glm::vec2 mouse = Input::GetMousePosition();
	const glm::vec2 mouseDelta = Input::GetMouseDelta();
	const glm::vec2 scroll = Input::GetScrollOffset();
	const glm::vec2 scrollDelta = Input::GetScrollDelta();

	ImGui::Text("WantCaptureMouse: %s", io.WantCaptureMouse ? "true" : "false");
	ImGui::Text("WantCaptureKeyboard: %s", io.WantCaptureKeyboard ? "true" : "false");
	ImGui::Separator();
	ImGui::Text("Mouse Position: (%.1f, %.1f)", mouse.x, mouse.y);
	ImGui::Text("Mouse Delta: (%.2f, %.2f)", mouseDelta.x, mouseDelta.y);
	ImGui::Text("Scroll Offset: (%.2f, %.2f)", scroll.x, scroll.y);
	ImGui::Text("Scroll Delta: (%.2f, %.2f)", scrollDelta.x, scrollDelta.y);
}

void StatisticsWindow::DrawSystemPanel() const {
	const auto& app = Application::Get();
	const auto& spec = app.GetSpecification();

	ImGui::Text("Window: %ux%u", app.GetWidth(), app.GetHeight());
	ImGui::Text("VSync: %s", spec.VSync ? "Enabled" : "Disabled");
	ImGui::Text("Frame Lock: %s", spec.LockFramerate ? "Enabled" : "Disabled");
	ImGui::Text("Target FPS: %d", spec.TargetFrameRate);
	ImGui::Text("Screen Mode: %s", spec.ScreenMode == ScreenMode::Windowed ? "Windowed" : (spec.ScreenMode == ScreenMode::Fullscreen ? "Fullscreen" : "Borderless"));
	ImGui::Separator();
	ImGui::Text("Renderer GPU Timing: %s", m_Snapshot.Renderer.GPUTimingAvailable ? "Available" : "Unavailable");
	ImGui::Text("Geometry GPU: %.3f ms", m_Snapshot.Renderer.GeometryPassGPUTimeMs);
	ImGui::Text("Background GPU: %.3f ms", m_Snapshot.Renderer.BackgroundPassGPUTimeMs);
	ImGui::Text("Lighting GPU: %.3f ms", m_Snapshot.Renderer.LightingPassGPUTimeMs);
	ImGui::Text("Total GPU: %.3f ms", m_Snapshot.Renderer.TotalGPUTimeMs);
}

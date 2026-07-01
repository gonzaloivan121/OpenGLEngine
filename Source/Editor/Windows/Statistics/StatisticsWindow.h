#pragma once

#include "Editor/Window.h"
#include "Core/Application/Application.h"
#include "Renderer/Renderer.h"
#include "ECS/Scene.h"

#include <vector>

class StatisticsWindow : public Window {
public:
	enum class Panel {
		Performance = 0,
		Renderer,
		Scene,
		Lighting,
		Memory,
		Audio,
		Input,
		System
	};

	struct SceneStats {
		uint32_t EntityCount = 0;
		uint32_t TransformCount = 0;
		uint32_t CameraCount = 0;
		uint32_t PrimaryCameraCount = 0;
		uint32_t MeshCount = 0;
		uint32_t MaterialCount = 0;
		uint32_t LightCount = 0;
		uint32_t AudioCount = 0;

		uint32_t DirectionalLights = 0;
		uint32_t PointLights = 0;
		uint32_t SpotLights = 0;

		uint64_t MeshVertices = 0;
		uint64_t MeshTriangles = 0;
	};

	struct Snapshot {
		ApplicationRuntimeStats Application;
		Renderer::RendererStats Renderer;
		SceneStats Scene;
	};

	StatisticsWindow(bool& isOpen, Scene& scene);

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;

private:
	void CollectSnapshot();
	SceneStats CollectSceneStats() const;
	void PushHistorySample(float frameTimeMs, float fps);
	void ApplyHistorySizeLimit();
	void EnsureActivePanelVisible();

	void DrawControls(bool& settingsDirty);
	void DrawDashboard() const;
	void DrawNavigation(bool& settingsDirty);
	void DrawDetailPanel() const;

	void DrawPerformancePanel() const;
	void DrawRendererPanel() const;
	void DrawScenePanel() const;
	void DrawLightingPanel() const;
	void DrawMemoryPanel() const;
	void DrawAudioPanel() const;
	void DrawInputPanel() const;
	void DrawSystemPanel() const;

private:
	Scene& m_Scene;

	Snapshot m_Snapshot;
	std::vector<float> m_FrameTimeHistory;
	std::vector<float> m_FPSHistory;

	float m_SampleTimerMs = 0.0f;
	bool m_Paused = false;
	Panel m_ActivePanel = Panel::Performance;
};
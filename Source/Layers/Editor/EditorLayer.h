#pragma once

#include "Core/Core.h"
#include "Core/Layer.h"
#include "Core/Settings/Settings.h"

#include "Editor/Window.h"
#include "ECS/Scene.h"

#include <vector>
#include <filesystem>

class EditorLayer : public Layer {
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
private:
	void DrawMenuBar();
	void DrawScenesRecursive(const std::filesystem::path& directoryPath);

	bool NewScene(const std::filesystem::path& filepath);
	bool CreateSceneAsset(const std::filesystem::path& filepath);
	bool SaveScene(const std::filesystem::path& filepath);
	bool LoadScene(const std::filesystem::path& filepath);

	bool NewMaterial(const std::filesystem::path& filepath);

	void ExportFrameAsImage();

	std::filesystem::path BuildExportPath(const std::filesystem::path& folder, const std::string& extension);
	void CheckOrCreateFolder(const std::filesystem::path& filepath);

	void HandleKeyboardShortcuts();
	void UpdateWindowTitle(const std::filesystem::path& filepath);
	void AddToRecentScenes(const std::filesystem::path& filepath);
private:
	bool m_RequestExport = false;

	WindowsSettings m_LastWindowsSettings;

	Scene m_Scene;
	UUID m_SelectedEntityID = UUID(0);

	// Scene Loading Data
	const std::filesystem::path m_DefaultSceneFilepath = "Internal/Scenes/Default.scene";
	const std::filesystem::path m_ScenesFilepath = "Internal/Scenes/";

	std::filesystem::path m_CurrentSceneFilepath;
	std::filesystem::path m_SceneFilepathToLoad;

	std::vector<std::filesystem::path> m_RecentSceneFilepaths;

	// Windows Vector
	std::vector<Scope<Window>> m_Windows;
};
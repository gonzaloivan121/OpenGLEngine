#include "EditorLayer.h"

#include "Core/Application/Application.h"
#include "Core/Log/Log.h"
#include "Core/Settings/SettingsManager.h"
#include "Core/Input/Input.h"

#include "ECS/Serialization/Scene/SceneSerializer.h"
#include "ECS/Serialization/Material/MaterialSerializer.h"
#include "ECS/Serialization/Shader/ShaderSerializer.h"

#include "Renderer/Renderer.h"

#include "Editor/Windows.h"
#include "Editor/UI.h"

#include <cstring>

EditorLayer::EditorLayer() {
	OnCreate();
}

void EditorLayer::OnCreate() {
	Log::Trace("EditorLayer::OnCreate - Creating Editor Layer");

	m_Windows.emplace_back(CreateScope<AboutWindow>(SettingsManager::Get().Editor.Windows.ShowAbout));
	m_Windows.emplace_back(CreateScope<InspectorWindow>(SettingsManager::Get().Editor.Windows.ShowInspector, m_Scene, m_SelectedEntityID));

	auto projectWindow = CreateScope<ProjectWindow>(SettingsManager::Get().Editor.Windows.ShowProject);
	projectWindow->SetSceneLoadCallback([this](const std::filesystem::path& filepath) {
		LoadScene(filepath);
	});
	projectWindow->SetSceneNewCallback([this](const std::filesystem::path& filepath) {
		CreateSceneAsset(filepath);
	});
	projectWindow->SetMaterialNewCallback([this](const std::filesystem::path& filepath) {
		CreateMaterialAsset(filepath);
	});
	projectWindow->SetShaderNewCallback([this](const std::filesystem::path& filepath) {
		CreateShaderAsset(filepath);
	});
	m_Windows.emplace_back(std::move(projectWindow));

	m_Windows.emplace_back(CreateScope<SceneWindow>(SettingsManager::Get().Editor.Windows.ShowScene, m_Scene, m_SelectedEntityID));
	m_Windows.emplace_back(CreateScope<SettingsWindow>(SettingsManager::Get().Editor.Windows.ShowSettings));
	m_Windows.emplace_back(CreateScope<StatisticsWindow>(SettingsManager::Get().Editor.Windows.ShowStatistics));

	auto viewportWindow = CreateScope<ViewportWindow>(SettingsManager::Get().Editor.Windows.ShowViewport, m_Scene);
	viewportWindow->SetSceneLoadCallback([this](const std::filesystem::path& filepath) {
		LoadScene(filepath);
	});
	m_Windows.emplace_back(std::move(viewportWindow));
}

void EditorLayer::OnAttach() {
	Log::Trace("EditorLayer::OnAttach - Attaching Editor Layer");

	for (const auto& window : m_Windows) {
		window->OnAttach();
	}

	const auto& settings = SettingsManager::Get();

	m_LastWindowsSettings = settings.Editor.Windows;

	if (!settings.Application.StartupScene.empty()) {
		LoadScene(settings.Application.StartupScene);
	} else {
		LoadScene(m_DefaultSceneFilepath);
	}
}

void EditorLayer::OnDetach() {
	Log::Trace("EditorLayer::OnDetach - Detaching Editor Layer");

	for (const auto& window : m_Windows) {
		window->OnDetach();
	}

	m_Windows.clear();
}

void EditorLayer::OnUpdate(Timestep ts) {
	// Auto-save when any window visibility changes.
	const auto& currentWindows = SettingsManager::Get().Editor.Windows;
	if (std::memcmp(&currentWindows, &m_LastWindowsSettings, sizeof(WindowsSettings)) != 0) {
		m_LastWindowsSettings = currentWindows;
		SettingsManager::Save();
	}

	for (const auto& window : m_Windows) {
		window->OnUpdate(ts);
	}

	HandleKeyboardShortcuts();

	if (m_RequestExport) {
		ExportFrameAsImage();
		m_RequestExport = false;
	}
}

void EditorLayer::OnUIRender() {
	m_SceneFilepathToLoad.clear();

	DrawMenuBar();

	for (const auto& window : m_Windows) {
		window->OnUIRender();
	}

	if (!m_SceneFilepathToLoad.empty()) {
		LoadScene(m_SceneFilepathToLoad);
	}

}

void EditorLayer::DrawMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		// File Menu
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "Ctrl+N")) {

			}

			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				SaveScene(m_CurrentSceneFilepath);
			}

			if (ImGui::BeginMenu("Open Recent", !m_RecentSceneFilepaths.empty())) {
				for (const auto& sceneFilepath : m_RecentSceneFilepaths) {
					if (!sceneFilepath.filename().empty()) {
						if (ImGui::MenuItem(sceneFilepath.filename().replace_extension("").string().c_str())) {
							m_SceneFilepathToLoad = sceneFilepath;
							break;
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scenes")) {
				DrawScenesRecursive(m_ScenesFilepath);

				ImGui::EndMenu();
			}

			UI::Separator();

			ImGui::MenuItem("Settings", "Ctrl+,", &SettingsManager::Get().Editor.Windows.ShowSettings);

			UI::Separator();

			if (ImGui::MenuItem("Exit", "Alt+F4")) {
				Application::Get().Close();
			}

			ImGui::EndMenu();
		}

		// Edit Menu
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			if (ImGui::MenuItem("Delete", "DEL")) {}
			ImGui::EndMenu();
		}

		// Export Menu
		if (ImGui::BeginMenu("Export")) {
			const auto& format = SettingsManager::Get().Export.ImageFormat;

			std::string imageLabel = "Image (." + [&format]() -> std::string {
				switch (format) {
					case ExportImageFormat::JPEG: return "jpg";
					case ExportImageFormat::BMP:  return "bmp";
					default:                      return "png";
				}
			}() + ")";

			if (ImGui::MenuItem(imageLabel.c_str())) {
				m_RequestExport = true;
			}

			UI::Tooltip("Export the current frame as an image to the 'Export/Image' folder.");

			ImGui::EndMenu();
		}

		// View Menu
		if (ImGui::BeginMenu("View")) {
			auto& windowsSettings = SettingsManager::Get().Editor.Windows;

			ImGui::MenuItem("About",     "F1",     &windowsSettings.ShowAbout);
			ImGui::MenuItem("Inspector", "Ctrl+I", &windowsSettings.ShowInspector);
			ImGui::MenuItem("Project",   "Ctrl+P", &windowsSettings.ShowProject);
			ImGui::MenuItem("Scene",	 "Ctrl+;", &windowsSettings.ShowScene);
			ImGui::MenuItem("Settings",  "Ctrl+,", &windowsSettings.ShowSettings);
			ImGui::MenuItem("Statistics","Ctrl+T", &windowsSettings.ShowStatistics);
			ImGui::MenuItem("Viewport",  "Ctrl+V", &windowsSettings.ShowViewport);

			ImGui::EndMenu();
		}

		// Help Menu
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("Documentation")) {
				UI::OpenLink("https://github.com/gonzaloivan121");
			}

			UI::Tooltip("Open the app documentation.");

			if (ImGui::MenuItem("About")) {
				SettingsManager::Get().Editor.Windows.ShowAbout = true;
			}

			UI::Tooltip("Open the about window.");

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void EditorLayer::DrawScenesRecursive(const std::filesystem::path& directoryPath) {
	for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
		const auto& path = entry.path();

		if (entry.is_directory()) {
			// If the entry is a directory, create a SUB-MENU for it
			if (ImGui::BeginMenu(path.filename().string().c_str())) {
				// And call this same function to fill that sub-menu
				DrawScenesRecursive(path);
				ImGui::EndMenu(); // Important to close the sub-menu
			}
		} else {
			// If it is a file, check that it is a scene
			if (!path.filename().empty() && path.extension() == ".scene") {
				// Create a normal `MenuItem`. Use replace_extension for a cleaner name.
				if (ImGui::MenuItem(path.filename().replace_extension("").string().c_str())) {
					m_SceneFilepathToLoad = path;
				}
			}
		}
	}
}

bool EditorLayer::NewScene(const std::filesystem::path& filepath) {
	Log::Info("EditorLayer::NewScene - Creating new Scene");

	m_CurrentSceneFilepath.clear();
	m_Scene.DestroyAllEntities();

	UpdateWindowTitle(filepath);

	if (SaveScene(filepath)) {
		Log::Info("EditorLayer::NewScene - New Scene has been created");

		return true;
	}

	Log::Warning("EditorLayer::NewScene - Couldn't create new Scene");

	return false;
}

bool EditorLayer::SaveScene(const std::filesystem::path& filepath) {
	Log::Info("EditorLayer::SaveScene - Saving Scene to " + filepath.string());

	SceneSerializer serializer(m_Scene);

	if (serializer.Serialize(filepath)) {
		Log::Info("EditorLayer::SaveScene - Scene has been saved");

		return true;
	}
	
	Log::Warning("EditorLayer::SaveScene - Couldn't save Scene");

	return false;
}

bool EditorLayer::LoadScene(const std::filesystem::path& filepath) {
	if (m_CurrentSceneFilepath == filepath) {
		Log::Warning("EditorLayer::LoadScene - Scene is already loaded: " + filepath.string());
		return false;
	}

	Log::Info("EditorLayer::LoadScene - Loading Scene from " + filepath.string());

	m_Scene.DestroyAllEntities();
	m_SelectedEntityID = UUID(0);

	SceneSerializer serializer(m_Scene);

	if (serializer.Deserialize(filepath)) {
		Log::Info("EditorLayer::LoadScene - Scene has been loaded");

		m_CurrentSceneFilepath = filepath;

		UpdateWindowTitle(filepath);
		AddToRecentScenes(filepath);

		return true;
	}

	Log::Warning("EditorLayer::LoadScene - Couldn't load Scene: " + filepath.string());

	return false;
}

bool EditorLayer::CreateSceneAsset(const std::filesystem::path& filepath) {
	Log::Trace("EditorLayer::CreateSceneAsset - Creating Scene asset at " + filepath.string());

	Scene scene;
	SceneSerializer serializer(scene);

	if (serializer.Serialize(filepath)) {
		Log::Info("EditorLayer::CreateSceneAsset - Scene asset created");
		return true;
	}

	Log::Warning("EditorLayer::CreateSceneAsset - Couldn't create Scene asset");
	return false;
}

bool EditorLayer::CreateMaterialAsset(const std::filesystem::path& filepath) {
	Log::Trace("EditorLayer::CreateMaterialAsset - Creating Material asset at " + filepath.string());

	Material material;
	MaterialSerializer serializer(material);

	if (serializer.Serialize(filepath)) {
		Log::Info("EditorLayer::CreateMaterialAsset - Material asset created");
		return true;
	}

	Log::Warning("EditorLayer::CreateMaterialAsset - Couldn't create Material asset");
	return false;
}

bool EditorLayer::CreateShaderAsset(const std::filesystem::path& filepath) {
	Log::Trace("EditorLayer::CreateShaderAsset - Creating Shader asset at " + filepath.string());

	ShaderAsset shader;
	ShaderSerializer serializer(shader);

	if (serializer.Serialize(filepath)) {
		Log::Info("EditorLayer::CreateShaderAsset - Shader asset created");
		return true;
	}

	Log::Warning("EditorLayer::CreateShaderAsset - Couldn't create Shader asset");
	return false;
}

std::filesystem::path EditorLayer::BuildExportPath(const std::filesystem::path& folder, const std::string& extension) {
	const std::filesystem::path exportRoot = SettingsManager::Get().Export.Folder;
	CheckOrCreateFolder(exportRoot);
	CheckOrCreateFolder(folder);

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << "Scene-" << std::put_time(std::localtime(&in_time_t), "%Y%m%d-%H%M%S") << extension;

	return folder / ss.str();
}

void EditorLayer::ExportFrameAsImage() {
	const auto& exportSettings = SettingsManager::Get().Export;

	std::string ext;
	switch (exportSettings.ImageFormat) {
		case ExportImageFormat::JPEG:	ext = ".jpg";	break;
		case ExportImageFormat::BMP:	ext = ".bmp";	break;
		case ExportImageFormat::PNG:
		default:						ext = ".png";	break;
	}

	const std::filesystem::path exportImageFolder = exportSettings.Folder / "Image";
	auto filepath = BuildExportPath(exportImageFolder, ext);
	Renderer::ExportFrame(filepath);
	Log::Info("EditorLayer::ExportFrameAsImage - Frame exported successfully to: " + filepath.string());
}

void EditorLayer::CheckOrCreateFolder(const std::filesystem::path& filepath) {
	if (!std::filesystem::exists(filepath)) {
		std::filesystem::create_directory(filepath);
	}
}

void EditorLayer::HandleKeyboardShortcuts() {
	if (Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl)) {
		if (Input::IsKeyDown(KeyCode::S)) {
			SaveScene(m_CurrentSceneFilepath);
		}

		if (Input::IsKeyDown(KeyCode::N)) {
			NewScene(m_CurrentSceneFilepath);
		}

		if (Input::IsKeyDown(KeyCode::P)) {
			SettingsManager::Get().Editor.Windows.ShowProject = !SettingsManager::Get().Editor.Windows.ShowProject;
		}

		if (Input::IsKeyDown(KeyCode::Comma)) {
			SettingsManager::Get().Editor.Windows.ShowSettings = !SettingsManager::Get().Editor.Windows.ShowSettings;
		}

		if (Input::IsKeyDown(KeyCode::Semicolon)) {
			SettingsManager::Get().Editor.Windows.ShowScene = !SettingsManager::Get().Editor.Windows.ShowScene;
		}

		if (Input::IsKeyDown(KeyCode::I)) {
			SettingsManager::Get().Editor.Windows.ShowInspector = !SettingsManager::Get().Editor.Windows.ShowInspector;
		}

		if (Input::IsKeyDown(KeyCode::T)) {
			SettingsManager::Get().Editor.Windows.ShowStatistics = !SettingsManager::Get().Editor.Windows.ShowStatistics;
		}

		if (Input::IsKeyDown(KeyCode::V)) {
			SettingsManager::Get().Editor.Windows.ShowViewport = !SettingsManager::Get().Editor.Windows.ShowViewport;
		}
	}

	if (Input::IsKeyDown(KeyCode::F1)) {
		SettingsManager::Get().Editor.Windows.ShowAbout = !SettingsManager::Get().Editor.Windows.ShowAbout;
	}
}

void EditorLayer::UpdateWindowTitle(const std::filesystem::path& filepath) {
	Application::Get().SetWindowTitle(filepath.filename().replace_extension("").string());
}

void EditorLayer::AddToRecentScenes(const std::filesystem::path& filepath) {
	auto it = std::find(m_RecentSceneFilepaths.begin(), m_RecentSceneFilepaths.end(), filepath);
	if (it != m_RecentSceneFilepaths.end()) {
		m_RecentSceneFilepaths.erase(it);
	}

	m_RecentSceneFilepaths.push_back(filepath);
}

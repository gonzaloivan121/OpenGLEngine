#pragma once

#include "Editor/Window.h"

#include "Core/Settings/Settings.h"

#include <vector>
#include <unordered_map>
#include <string>

class SettingsWindow : public Window {
public:
	SettingsWindow(bool& isOpen) : Window(isOpen) {}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;
private:
	void DrawSettings();

	void DrawApplicationSettings(ApplicationSettings& applicationSettings);
	void DrawAudioSettings(AudioSettings& audioSettings);
	void DrawEditorSettings(EditorSettings& editorSettings);
	void DrawSceneCameraSettings(SceneCameraSettings& sceneCameraSettings);
	void DrawExportSettings(ExportSettings& exportSettings);
	void DrawNavigationSettings(NavigationSettings& navigationSettings);
	void DrawRenderingSettings(RenderingSettings& renderingSettings);

	void DrawWIP();
private:
	std::vector<Category> m_Categories;
	Category m_CurrentCategory = Category::Application;

	std::vector<EditorTheme> m_Themes;
	std::vector<RenderingEngine> m_RenderingEngines;
	std::vector<WindowMode> m_WindowModes;
	std::vector<ExportImageFormat> m_ExportImageFormats;

	std::vector<SampleRate> m_SampleRates;
	std::vector<BufferSize> m_BufferSizes;
};
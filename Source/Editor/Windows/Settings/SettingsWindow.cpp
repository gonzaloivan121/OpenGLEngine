#include "SettingsWindow.h"

#include "Core/Application/Application.h"
#include "Core/Audio/AudioEngine.h"
#include "Core/Log/Log.h"
#include "Core/Settings/Manager/SettingsManager.h"

#include "Utilities/Utilities.h"

#include "Editor/UI.h"
#include "Editor/Theme/ThemeManager.h"

#include <algorithm>

void SettingsWindow::OnCreate() {
	Log::Trace("SettingsWindow::OnCreate - Creating Settings Window");
}

void SettingsWindow::OnAttach() {
	Log::Trace("SettingsWindow::OnAttach - Attaching Settings Window");

	m_Categories = {
		Category::Application,
		Category::Audio,
		Category::Editor,
		Category::Export,
		Category::Graphics,
		Category::Input,
		Category::Localization,
		Category::Quality,
		Category::Rendering,
		Category::Time
	};

	m_Themes = {
		EditorTheme::AdobeInspired,
		EditorTheme::AyuDark,
		EditorTheme::BessDark,
		EditorTheme::BlackDevil,
		EditorTheme::BootstrapDark,
		EditorTheme::Carbon,
		EditorTheme::Cherno,
		EditorTheme::Cherry,
		EditorTheme::ClassicSteam,
		EditorTheme::Classic,
		EditorTheme::ComfortableDarkCyan,
		EditorTheme::ComfortableLightOrange,
		EditorTheme::Comfy,
		EditorTheme::Darcula,
		EditorTheme::DarkRed,
		EditorTheme::DarkRuda,
		EditorTheme::Dark,
		EditorTheme::Darky,
		EditorTheme::DeepDark,
		EditorTheme::DiscordDark,
		EditorTheme::Enemymouse,
		EditorTheme::Everforest,
		EditorTheme::Excellency,
		EditorTheme::FutureDark,
		EditorTheme::Gold,
		EditorTheme::GreenFont,
		EditorTheme::GreenLeaf,
		EditorTheme::HazyDark,
		EditorTheme::LedSynthmaster,
		EditorTheme::Light,
		EditorTheme::MaterialFlat,
		EditorTheme::Microsoft,
		EditorTheme::Modern,
		EditorTheme::Photoshop,
		EditorTheme::PurpleComfy,
		EditorTheme::QuickMinimalLook,
		EditorTheme::RedFont,
		EditorTheme::RedOni,
		EditorTheme::Rest,
		EditorTheme::RoundedVisualStudio,
		EditorTheme::SoftCherry,
		EditorTheme::SonicRiders,
		EditorTheme::Unreal,
		EditorTheme::VisualStudio,
		EditorTheme::Windark
	};

	m_RenderingEngines = {
		RenderingEngine::OpenGL,
		RenderingEngine::DirectX,
		RenderingEngine::Vulkan
	};

	m_WindowModes = {
		WindowMode::Windowed,
		WindowMode::Fullscreen,
		WindowMode::Borderless
	};

	m_ExportImageFormats = {
		ExportImageFormat::PNG,
		ExportImageFormat::JPEG,
		ExportImageFormat::BMP
	};

	m_SampleRates = {
		SampleRate::SR_44100,
		SampleRate::SR_48000,
		SampleRate::SR_88200,
		SampleRate::SR_96000,
		SampleRate::SR_176400,
		SampleRate::SR_192000};

	m_BufferSizes = {
		BufferSize::BS_16,
		BufferSize::BS_32,
		BufferSize::BS_48,
		BufferSize::BS_64,
		BufferSize::BS_96,
		BufferSize::BS_128,
		BufferSize::BS_160,
		BufferSize::BS_192,
		BufferSize::BS_256,
		BufferSize::BS_512,
		BufferSize::BS_1024};
}

void SettingsWindow::OnDetach() {
	Log::Trace("SettingsWindow::OnDetach - Detaching Settings Window");
}

void SettingsWindow::OnUpdate(Timestep ts) {}

void SettingsWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	if (ImGui::Begin("Settings", &m_IsOpen)) {
		if (ImGui::BeginTable("##SettingsWindowTable", 2, ImGuiTableFlags_Resizable)) {
			if (ImGui::TableNextColumn()) {
				if (ImGui::BeginChild("##SettingsSidebar", { 0, 0 }, ImGuiChildFlags_Borders)) {
					const auto& style = ImGui::GetStyle();
					for (const auto& category : m_Categories) {
						if (ImGui::Selectable(Utilities::CategoryToString(category).c_str(), m_CurrentCategory == category)) {
							m_CurrentCategory = category;
						}
					}
				}

				ImGui::EndChild();
			}

			if (ImGui::TableNextColumn()) {
				const auto& style = ImGui::GetStyle();
				float footerHeight =
					ImGui::GetFrameHeight()
					+ style.WindowPadding.y * 2.0f
					+ style.ChildBorderSize * 2.0f
					+ style.ItemSpacing.y * 2.0f;

				if (ImGui::BeginChild("##SettingsContent", { 0, -footerHeight }, ImGuiChildFlags_Borders)) {
					DrawSettings();
				}

				ImGui::EndChild();

				UI::Separator();

				if (ImGui::BeginChild("##SettingsFooter", { 0, 0 }, ImGuiChildFlags_Borders)) {
					float buttonWidth = 100.0f;
					float totalButtonWidth = buttonWidth * 2 + style.ItemSpacing.x;
					float indent = (ImGui::GetContentRegionAvail().x - totalButtonWidth) * 0.5f;

					if (indent > 0) {
						ImGui::Indent(indent);
					}

					if (UI::Button("Save", { buttonWidth , 0 })) {
						SettingsManager::Save();
					}

					ImGui::SameLine();

					if (UI::Button("Load", { buttonWidth , 0 })) {
						SettingsManager::Load();
					}

					if (indent > 0) {
						ImGui::Unindent(indent);
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

void SettingsWindow::DrawSettings() {
	Settings& settings = SettingsManager::Get();

	switch (m_CurrentCategory) {
		case Category::Application: DrawApplicationSettings(settings.Application);	break;
		case Category::Audio:		DrawAudioSettings(settings.Audio);				break;
		case Category::Editor:		DrawEditorSettings(settings.Editor);			break;
		case Category::Input:		DrawNavigationSettings(settings.Navigation);	break;
		case Category::Rendering:	DrawRenderingSettings(settings.Rendering);		break;
		case Category::Export:		DrawExportSettings(settings.Export);			break;
		default:					DrawWIP();										break;
	}
}

void SettingsWindow::DrawApplicationSettings(ApplicationSettings& applicationSettings) {
	UI::DisabledInputText("Name", applicationSettings.Name);
	UI::Tooltip("The name of the Application.");

	UI::SceneSlot("Startup Scene", applicationSettings.StartupScene);
	UI::Tooltip("The startup scene.\nUse the Project Window to drag a Startup Scene from the Assets Folder.\nOr set it up manually through the 'Settings.yaml' file.");

	UI::Separator();

	const auto& versionSettings = applicationSettings.Version;

	std::string versionStr = versionSettings.GetVersion();

	UI::DisabledInputText("Version", versionStr);
	UI::Tooltip("The version of the Application.");

	UI::Separator();

	UI::Bool("Maximized", applicationSettings.Maximized);
	UI::Tooltip("Whether the Application will start Maximized or not.");

	UI::Bool("Debug Mode", applicationSettings.DebugMode);
	UI::Tooltip("When the Debug Mode is activated, additional diagnostic information may be shown.");

	UI::Bool("Log to File", applicationSettings.LogToFile);
	UI::Tooltip("When enabled, all log output is also written to 'Application.log' on disk.");

	UI::Bool("Escape Closes App", applicationSettings.EscapeClosesApp);
	UI::Tooltip("When enabled, pressing Escape will close the application.");

	UI::Separator();
}

void SettingsWindow::DrawAudioSettings(AudioSettings& audioSettings) {
	if (UI::CollapsingHeader("Device")) {
		auto& deviceSettings = audioSettings.Device;

		{
			const auto& inputDevices = AudioEngine::GetInputDevices();
			std::vector<const char*> deviceNames;
			int currentDeviceIndex = -1;
	
			for (int i = 0; i < inputDevices.size(); ++i) {
				deviceNames.push_back(inputDevices[i].Name.c_str());
				if (inputDevices[i].ID == deviceSettings.InputDeviceID) {
					currentDeviceIndex = i;
				}
			}
	
			int selection = currentDeviceIndex;
			if (UI::Dropdown("Input Device", deviceNames.data(), (int)deviceNames.size(), &selection)) {
				// If the user changes the selection, update the ID in the settings
				if (selection >= 0) {
					deviceSettings.InputDeviceID = inputDevices[selection].ID;
					//AudioEngine::Restart();
				}
			}
			UI::Tooltip("Select the input audio device to be used for capturing audio input.");
		}

		{
			const auto& outputDevices = AudioEngine::GetOutputDevices();
			std::vector<const char*> deviceNames;
			int currentDeviceIndex = -1;

			for (int i = 0; i < outputDevices.size(); ++i) {
				deviceNames.push_back(outputDevices[i].Name.c_str());
				if (outputDevices[i].ID == deviceSettings.OutputDeviceID) {
					currentDeviceIndex = i;
				}
			}

			int selection = currentDeviceIndex;
			if (UI::Dropdown("Output Device", deviceNames.data(), (int)deviceNames.size(), &selection)) {
				// If the user changes the selection, update the ID in the settings
				if (selection >= 0) {
					deviceSettings.OutputDeviceID = outputDevices[selection].ID;
					//AudioEngine::Restart();
				}
			}
		}

		UI::Dropdown("Sample Rate", m_SampleRates, deviceSettings.SampleRate, Utilities::SampleRateToString);
		UI::Tooltip("Select the sample rate for audio processing, which determines the number of audio samples captured or played back per second.");

		UI::Dropdown("Buffer Size", m_BufferSizes, deviceSettings.BufferSize, Utilities::BufferSizeToString);
		UI::Tooltip("Select the buffer size for audio processing, which determines the number of audio samples processed in each audio callback.");

		UI::Bool("Pass Through", deviceSettings.PassThrough);
		UI::Tooltip("When enabled, audio input will be passed through to the output, allowing users to hear their own voice or other input audio in real-time.");
	}

	if (UI::CollapsingHeader("Volume")) {
		auto& volumeSettings = audioSettings.Volume;

		UI::SliderFloat("Master", volumeSettings.Master, 0.0f, 1.0f);
		UI::Tooltip("The master volume level for the application.");

		UI::SliderFloat("Ambience", volumeSettings.Ambience, 0.0f, 1.0f);
		UI::Tooltip("The volume level for ambient sounds.");

		UI::SliderFloat("Effects", volumeSettings.Effects, 0.0f, 1.0f);
		UI::Tooltip("The volume level for sound effects.");

		UI::SliderFloat("Music", volumeSettings.Music, 0.0f, 1.0f);
		UI::Tooltip("The volume level for music playback.");

		UI::SliderFloat("Voices", volumeSettings.Voices, 0.0f, 1.0f);
		UI::Tooltip("The volume level for voice audio.");

		UI::Bool("Mute All", volumeSettings.MuteAll);
		UI::Tooltip("When enabled, all audio output will be muted.");

		UI::Separator();
	}
}

void SettingsWindow::DrawEditorSettings(EditorSettings& editorSettings) {
	if (UI::CollapsingHeader("Appearance")) {
		auto& appearanceSettings = editorSettings.Appearance;

		if (UI::Dropdown("Theme", m_Themes, appearanceSettings.Theme, Utilities::EditorThemeToString)) {
			if (ThemeManager::Load(ThemeManager::Select(appearanceSettings.Theme))) {
				ThemeManager::Apply(ThemeManager::Get());
			}
		}
		UI::Tooltip("Set the Editor Theme.");

		UI::DragInt("Font Size", appearanceSettings.FontSize, 0, 1638);
		UI::Tooltip("To change the Font Size you need to restart the Editor.");

		if (UI::DragFloat("UI Scale", appearanceSettings.UIScale, 0.01f, 10.0f)) {
			auto& style = ImGui::GetStyle();
			style.FontScaleMain = appearanceSettings.UIScale;
		}
		UI::Tooltip("Change the UI Scale.");

		UI::DragFloat("Column Width", appearanceSettings.ColumnWidth, 0.0f, 300.0f);
		UI::Tooltip("Changes the column width for the Property Grid Name Column");

		if (UI::CollapsingHeader("Project Window")) {
			auto& projectWindowSettings = appearanceSettings.ProjectWindow;

			UI::SliderFloat("Icon Size", projectWindowSettings.IconSize, 16.0f, 300.0f);
			UI::Tooltip("Changes the size of the icons in the Project Window.");

			UI::SliderFloat("Icon Padding", projectWindowSettings.IconPadding, 24.0f, 100.0f);
			UI::Tooltip("Changes the padding between icons in the Project Window.");
			
			UI::Separator();
		}

		if (UI::CollapsingHeader("Scene Camera")) {
			DrawSceneCameraSettings(editorSettings.SceneCamera);
		}
	}

	UI::DragInt("Auto Save Interval", editorSettings.AutoSaveInterval, 0, 86400);
	UI::Tooltip("Auto Save is not supported yet.");

	UI::Separator();
}

void SettingsWindow::DrawSceneCameraSettings(SceneCameraSettings& sceneCameraSettings) {
	UI::Vec3("Position", sceneCameraSettings.Position);
	UI::Tooltip("The live scene camera position used by the Scene viewport.");

	UI::DragFloat("Yaw", sceneCameraSettings.Yaw, -360.0f, 360.0f, 0.1f);
	UI::Tooltip("The horizontal rotation angle in degrees.");

	UI::DragFloat("Pitch", sceneCameraSettings.Pitch, -89.0f, 89.0f, 0.1f);
	UI::Tooltip("The vertical rotation angle in degrees.");

	UI::DragFloat("FOV", sceneCameraSettings.FOV, 1.0f, 120.0f, 0.1f);
	UI::Tooltip("Field of view in degrees.");

	UI::DragFloat("Near Clip", sceneCameraSettings.NearClip, 0.001f, 10.0f, 0.001f);
	UI::Tooltip("The near clipping plane.");

	UI::DragFloat("Far Clip", sceneCameraSettings.FarClip, 1.0f, 100000.0f, 1.0f);
	UI::Tooltip("The far clipping plane.");

	UI::Separator();

	UI::DragFloat("Movement Speed", sceneCameraSettings.MovementSpeed, 0.1f, 50.0f);
	UI::Tooltip("Pan speed when using WASD in the Scene viewport.");

	UI::DragFloat("Fast Movement Speed", sceneCameraSettings.FastMovementSpeed, 0.1f, 50.0f);
	UI::Tooltip("Speed multiplier for fast panning/movement (Shift key). Higher values move the view faster.");

	UI::DragFloat("Rotation Speed", sceneCameraSettings.RotationSpeed, 0.1f, 50.0f);
	UI::Tooltip("Rotation speed when using mouse look in the Scene viewport.");

	UI::DragFloat("Zoom Speed", sceneCameraSettings.ZoomSpeed, 0.1f, 50.0f);
	UI::Tooltip("Zoom speed when using the scroll wheel.");

	UI::Separator();

	UI::DragFloat("Smoothing", sceneCameraSettings.Smoothing, 0.1f, 50.0f);
	UI::Tooltip("Interpolation smoothing factor.");

	UI::Separator();

	UI::Bool("Invert Zoom", sceneCameraSettings.InvertZoom);
	UI::Tooltip("Reverses the scroll wheel zoom direction.");

	UI::Separator();
}

void SettingsWindow::DrawExportSettings(ExportSettings& exportSettings) {
	UI::Dropdown("Image Format", m_ExportImageFormats, exportSettings.ImageFormat, Utilities::ExportImageFormatToString);
	UI::Tooltip("The image format used when exporting a frame.");

	if (exportSettings.ImageFormat == ExportImageFormat::JPEG) {
		UI::SliderInt("Image Quality", exportSettings.ImageQuality, 0, 100);
		UI::Tooltip("JPEG compression quality (0 = smallest file, 100 = best quality).");
	}

	std::string folderStr = exportSettings.Folder.string();
	if (UI::InputText("Export Folder", folderStr)) {
		exportSettings.Folder = folderStr;
	}
	UI::Tooltip("Root folder where exported images and configurations are saved.");

	UI::Separator();
}

void SettingsWindow::DrawNavigationSettings(NavigationSettings& navigationSettings) {
	UI::DragFloat("Movement Speed", navigationSettings.MovementSpeed, 0.1f, 50.0f);
	UI::Tooltip("Pan speed when using WASD or dragging the viewport.");

	UI::DragFloat("Fast Movement Speed", navigationSettings.FastMovementSpeed, 0.1f, 50.0f);
	UI::Tooltip("Speed multiplier for fast panning/movement (Shift key). Higher values move the view faster.");

	UI::DragFloat("Rotation Speed", navigationSettings.RotationSpeed, 0.1f, 50.0f);
	UI::Tooltip("Rotation speed when using the Q and E keys.");

	UI::DragFloat("Zoom Speed", navigationSettings.ZoomSpeed, 0.1f, 50.0f);
	UI::Tooltip("Zoom speed when using the scroll wheel, Shift or Ctrl.");

	UI::Separator();

	UI::DragFloat("Smoothing", navigationSettings.Smoothing, 0.1f, 50.0f);
	UI::Tooltip("Interpolation smoothing factor.\nHigher values produce snappier, faster transitions.");

	UI::Separator();

	UI::Bool("Invert Zoom", navigationSettings.InvertZoom);
	UI::Tooltip("Reverses the scroll wheel zoom direction.");

	UI::Separator();
}

void SettingsWindow::DrawRenderingSettings(RenderingSettings& renderingSettings) {
	UI::Dropdown("Engine", m_RenderingEngines, renderingSettings.Engine, Utilities::RenderingEngineToString);
	UI::Tooltip("Select a Rendering API.\nCurrently, only OpenGL is supported.");

	UI::Dropdown("Window Mode", m_WindowModes, renderingSettings.Mode, Utilities::WindowModeToString);
	UI::Tooltip("Windowed: standard window.\nFullscreen: exclusive fullscreen.\nBorderless: borderless window covering the screen.");

	if (UI::CollapsingHeader("Resolution")) {
		auto& resolutionSettings = renderingSettings.Resolution;

		UI::DragInt("Width", resolutionSettings.Width, 1);
		UI::Tooltip("The width of the Application.");

		UI::DragInt("Height", resolutionSettings.Height, 1);
		UI::Tooltip("The height of the Application.");

		UI::Separator();

		UI::SliderFloat("Scale", resolutionSettings.Scale, 0.01f, 10.0f);
		UI::Tooltip("The scale of the Rendering on the Viewport Window.\nUse at your own risk.");

		UI::Separator();
	}

	UI::Bool("VSync", renderingSettings.VSync);
	UI::Tooltip("Whether VSync is activated or not.");

	UI::Bool("Lock Framerate", renderingSettings.LockFramerate);
	UI::Tooltip("When enabled, the frame rate will be capped to the value specified in 'Target Frame Rate'.");

	if (renderingSettings.LockFramerate) {
		UI::DragInt("Target Frame Rate", renderingSettings.TargetFrameRate, 1, 960);
		UI::Tooltip("Frame rate cap in FPS. Set to 0 for uncapped rendering.");
	}

	UI::Separator();
}

void SettingsWindow::DrawWIP() {
	ImGui::Text("Work in progress");
}

#pragma once

#include <string>
#include <filesystem>

#include <glm/glm.hpp>

/**
 * Represents the different categories of settings available in the application.
 * 
 * Each category corresponds to a specific aspect of the application's configuration, allowing for organized management of settings.
 */
enum class Category {
	/// @brief General settings related to the application, such as name, version, and startup configuration.
	Application,

	/// @brief Settings related to audio configuration, including input/output devices and volume levels.
	Audio,

	/// @brief Settings specific to the editor interface, including appearance and auto-save options.
	Editor,

	/// @brief Settings related to graphics rendering, such as resolution and rendering engine.
	Graphics,

	/// @brief Settings for input devices and controls, including navigation speeds and scroll behaviour.
	Input,

	/// @brief Settings for language and regional preferences.
	Localization,

	/// @brief Settings that affect the quality of graphics and performance.
	Quality,

	/// @brief Settings specific to the rendering process, including engine selection and resolution.
	Rendering,

	/// @brief Settings related to time management within the application, such as frame rate and time.
	Time,

	/// @brief Settings for image and scene export, including format, quality, and output folder.
	Export
};

/**
 * Represents the window display mode for the application.
 */
enum class WindowMode {
	/// @brief The application runs in a standard window with borders and title bar.
	Windowed,

	/// @brief The application runs in true fullscreen mode, taking over the display.
	Fullscreen,

	/// @brief The application runs in a borderless window that covers the entire screen.
	Borderless
};

/**
 * Represents the image format used when exporting frames.
 */
enum class ExportImageFormat {
	/// @brief Portable Network Graphics — lossless, best quality.
	PNG,

	/// @brief JPEG — lossy compression, smaller file sizes.
	JPEG,

	/// @brief Windows Bitmap — uncompressed, large files.
	BMP
};

/**
 * Represents the different rendering engines available for the application.
 * 
 * Each rendering engine corresponds to a specific graphics API that the application can utilize for rendering graphics.
 * The choice of rendering engine can affect performance, compatibility, and visual quality.
 */
enum class RenderingEngine {
	/// @brief A cross-platform graphics API that is widely supported and used for rendering 2D and 3D graphics.
	OpenGL,

	/// @brief A collection of APIs developed by Microsoft for handling tasks related to multimedia, especially game programming and video, primarily on Windows platforms.
	DirectX,

	/// @brief A low-overhead, cross-platform 3D graphics and compute API that provides high-efficiency access to modern GPUs, designed to offer better performance and more balanced CPU/GPU usage compared to older APIs like OpenGL and DirectX.
	Vulkan
};

/**
 * Represents the resolution settings for the application, including width, height, and scale.
 * 
 * The resolution settings determine the dimensions of the application's window and how the content is scaled within it.
 */
struct ResolutionSettings {
	/// @brief The width of the application window in pixels.
	int Width = 1920;

	/// @brief The height of the application window in pixels.
	int Height = 1080;

	/// @brief A scaling factor that can be applied to the content within the application window, allowing for adjustments to the size of UI elements and overall display without changing the actual resolution.
	float Scale = 1.0f;
};

/**
 * Represents the rendering settings for the application, including the rendering engine, resolution, window mode, and VSync.
 * 
 * The rendering settings determine how the application renders graphics and can affect performance and visual quality.
 */
struct RenderingSettings {
	/// @brief The rendering engine to be used for graphics rendering, which can be `OpenGL`, `DirectX`, or `Vulkan`.
	RenderingEngine Engine = RenderingEngine::OpenGL;

	/// @brief The resolution settings that specify the width, height, and scale of the application window.
	ResolutionSettings Resolution;

	/// @brief The window display mode: Windowed, Fullscreen, or Borderless.
	WindowMode Mode = WindowMode::Windowed;

	/// @brief A boolean value that indicates whether vertical synchronization (`VSync`) is enabled, which can help prevent screen tearing by synchronizing the frame rate of the application with the refresh rate of the monitor.
	bool VSync = true;

	/// @brief A boolean value that indicates whether to lock the frame rate to the target frame rate specified in `TargetFrameRate`.
	bool LockFramerate = false;

	/// @brief The target frame rate cap in frames per second.
	int TargetFrameRate = 0;
};

/**
 * Represents the version settings for the application, including major, minor, and patch version numbers.
 * 
 * The version settings provide information about the current version of the application, which can be useful for tracking updates, compatibility, and user support.
 */
struct VersionSettings {
	/// @brief The major version number, which typically indicates significant changes or milestones in the application's development.
	int Major = 1;

	/// @brief The minor version number, which usually indicates smaller updates, new features, or improvements that are added to the application without breaking compatibility.
	int Minor = 0;

	/// @brief The patch version number, which often indicates bug fixes, performance improvements, or minor changes that do not introduce new features or break compatibility.
	int Patch = 0;

	/**
	 * Gets the version as a string in the format `Major` `.` `Minor` `.` `Patch`.
	 * 
	 * @return A string representation of the version number.
	 */
	const std::string GetVersion() const {
		return (
			std::to_string(Major) + "." +
			std::to_string(Minor) + "." +
			std::to_string(Patch)
		);
	}

	/**
	 * Gets the version name as a string in the format `v` `Major` `.` `Minor` `.` `Patch`.
	 *
	 * @return A string representation of the version name.
	 */
	const std::string GetName() const {
		return "v" + GetVersion();
	}
};

/**
 * Represents the application settings for the application, including the name, startup configuration, version, and various options.
 * 
 * The application settings provide general configuration options for the application, such as its name, version, and behavior on startup.
 */
struct ApplicationSettings {
	/// @brief The name of the application, which can be displayed in the title bar and used for identification purposes.
	std::string Name = "OpenGLEngine";

	/// @brief The file path to the startup scene, which specifies the initial settings or state of the application when it is launched.
	std::filesystem::path StartupScene = "Internal/Scenes/Default.scene";

	/// @brief The version settings that provide information about the current version of the application.
	VersionSettings Version;

	/// @brief A boolean value that indicates whether the application should start in a maximized state, filling the entire screen.
	bool Maximized = true;

	/// @brief A boolean value that indicates whether the application is running in debug mode, which can enable additional logging, error reporting, or debugging features for development purposes.
	bool DebugMode = false;

	/// @brief A boolean value that indicates whether pressing the Escape key should close the application, providing a quick way for users to exit the application using the keyboard.
	bool EscapeClosesApp = false;

	/// @brief A boolean value that indicates whether the application should write its log output to a file on disk.
	bool LogToFile = false;
};

struct VolumeSettings {
	/// @brief The master volume level for the application, which controls the overall audio output.
	float Master = 1.0f;

	/// @brief The volume level for ambient sounds in the application, which can include background music or environmental sounds.
	float Ambience = 1.0f;

	/// @brief The volume level for sound effects in the application, which can include game sounds, notifications, or other audio cues.
	float Effects = 1.0f;

	/// @brief The volume level for music in the application, which can include background tracks or theme music.
	float Music = 1.0f;

	/// @brief The volume level for voice audio in the application, which can include character dialogue or voiceovers.
	float Voices = 1.0f;

	/// @brief A boolean value that indicates whether all audio output should be muted, effectively silencing the application.
	bool MuteAll = false;
};

/**
 * Represents the device settings for audio input/output, including device IDs, sample rate, buffer size, and pass-through option.
 */
enum class SampleRate {
	SR_44100 = 44100,
	SR_48000 = 48000,
	SR_88200 = 88200,
	SR_96000 = 96000,
	SR_176400 = 176400,
	SR_192000 = 192000
};

/**
 * Represents the buffer size settings for audio processing, which determine the number of audio samples processed in each audio callback.
 */
enum class BufferSize {
	BS_16 = 16,
	BS_32 = 32,
	BS_48 = 48,
	BS_64 = 64,
	BS_96 = 96,
	BS_128 = 128,
	BS_160 = 160,
	BS_192 = 192,
	BS_256 = 256,
	BS_512 = 512,
	BS_1024 = 1024
};

/**
 * Represents the settings for audio input/output devices, sample rate, buffer size, and pass-through option.
 */
struct DeviceSettings {
	/// @brief The ID of the input audio device to be used for capturing audio input, such as a microphone.
	int InputDeviceID = -1;

	/// @brief The ID of the output audio device to be used for audio playback, such as speakers or headphones.
	int OutputDeviceID = -1;

	/// @brief The sample rate for audio processing, which determines the number of audio samples captured or played back per second.
	SampleRate SampleRate = SampleRate::SR_48000;

	/// @brief The buffer size for audio processing, which determines the number of audio samples processed in each audio callback.
	BufferSize BufferSize = BufferSize::BS_256;

	/// @brief A boolean value that indicates whether audio input should be passed through to the output, allowing users to hear their own voice or other input audio in real-time.
	bool PassThrough = false;
};

struct AudioSettings {
	/// @brief The volume settings that provide control over the master volume and individual audio category volumes, allowing users to adjust the audio output levels for different types of sounds in the application.
	VolumeSettings Volume;

	/// @brief The device settings that provide configuration options for audio input/output devices, sample rate, and buffer size, allowing users to select their preferred audio hardware and performance settings.
	DeviceSettings Device;
};

/**
 * Represents the different editor themes available for the application.
 * 
 * Each editor theme corresponds to a specific visual style for the editor interface, allowing users to customize the appearance of the editor according to their preferences.
 */
enum class EditorTheme {
	AdobeInspired,
	AyuDark,
	BessDark,
	BlackDevil,
	BootstrapDark,
	Carbon,
	Cherno,
	Cherry,
	ClassicSteam,
	Classic,
	ComfortableDarkCyan,
	ComfortableLightOrange,
	Comfy,
	Darcula,
	DarkRed,
	DarkRuda,
	Dark,
	Darky,
	DeepDark,
	DiscordDark,
	Enemymouse,
	Everforest,
	Excellency,
	FutureDark,
	Gold,
	GreenFont,
	GreenLeaf,
	HazyDark,
	LedSynthmaster,
	Light,
	MaterialFlat,
	Microsoft,
	Modern,
	Photoshop,
	PurpleComfy,
	QuickMinimalLook,
	RedFont,
	RedOni,
	Rest,
	RoundedVisualStudio,
	SoftCherry,
	SonicRiders,
	Unreal,
	VisualStudio,
	Windark
};

/**
 * Represents the settings specific to the Project window in the editor, including icon size and padding.
 * 
 * The Project window settings allow users to customize the appearance and layout of the Project window in the editor interface, affecting how project assets are displayed and organized.
 */
struct ProjectWindowSettings {
	/// @brief The size of project icons displayed in the editor interface, which can affect the visibility and appearance of project assets.
	float IconSize = 96.0f;

	/// @brief The padding between project icons in the editor interface, which can affect the layout and spacing of project assets.
	float IconPadding = 24.0f;
};

/**
 * Represents the appearance settings for the editor, including the theme, font size, UI scale, and column width.
 * 
 * The appearance settings allow users to customize the visual aspects of the editor interface, such as the color scheme, font size, and layout, to enhance their user experience and productivity.
 */
struct AppearanceSettings {
	/// @brief The editor theme to be used for the editor interface, which can be one of the predefined themes.
	EditorTheme Theme = EditorTheme::Excellency;

	/// @brief The font size used in the editor interface, which can affect the readability and overall appearance of text elements within the editor.
	int FontSize = 14;

	/// @brief A scaling factor that can be applied to the user interface (UI) elements within the editor, allowing users to adjust the size of UI components for better visibility or to fit their screen resolution.
	float UIScale = 1.0f;

	/// @brief The width of columns in the editor interface, which can affect the layout and organization of content within the editor.
	float ColumnWidth = 200.0f;

	/// @brief The settings specific to the Project window in the editor, including icon size and padding.
	ProjectWindowSettings ProjectWindow;
};

/**
 * Represents the editor scene camera settings used by the Scene window.
 *
 * These values control the live camera pose and navigation behavior inside the Scene view.
 */
struct SceneCameraSettings {
	/// @brief The current world-space camera position.
	glm::vec3 Position = glm::vec3(0.0f, 1.5f, 5.0f);

	/// @brief The current yaw angle in degrees.
	float Yaw = -90.0f;

	/// @brief The current pitch angle in degrees.
	float Pitch = -10.0f;

	/// @brief The field of view in degrees.
	float FOV = 60.0f;

	/// @brief The near clipping plane.
	float NearClip = 0.1f;

	/// @brief The far clipping plane.
	float FarClip = 1000.0f;

	/// @brief Speed multiplier for panning/movement. Higher values move the view faster.
	float MovementSpeed = 2.0f;

	/// @brief Speed multiplier for fast panning/movement (Shift key). Higher values move the view faster.
	float FastMovementSpeed = 5.0f;

	/// @brief Speed multiplier for rotation (mouse look). Higher values rotate faster.
	float RotationSpeed = 2.0f;

	/// @brief Speed multiplier for zooming (scroll wheel). Higher values zoom faster.
	float ZoomSpeed = 2.0f;

	/// @brief Smoothing factor for parameter interpolation. Higher values produce snappier, faster transitions.
	float Smoothing = 5.0f;

	/// @brief When enabled, the scroll wheel zoom direction is reversed.
	bool InvertZoom = false;
};

/**
 * Represents the visibility of editor windows.
 */
struct WindowsSettings {
	/// @brief Whether the About window is visible.
	bool ShowAbout = false;

	/// @brief Whether the Game window is visible.
	bool ShowGame = true;

	/// @brief Whether the Hierarchy window is visible.
	bool ShowHierarchy = true;

	/// @brief Whether the Inspector window is visible.
	bool ShowInspector = true;

	/// @brief Whether the Project window is visible.
	bool ShowProject = false;

	/// @brief Whether the Scene window is visible.
	bool ShowScene = true;

	/// @brief Whether the Settings window is visible.
	bool ShowSettings = false;

	/// @brief Whether the Statistics window is visible.
	bool ShowStatistics = false;
};

/**
 * Represents user preferences for the Statistics window.
 */
struct StatisticsWindowSettings {
	/// @brief Sampling interval in milliseconds used by the statistics window aggregator.
	float SamplingIntervalMs = 250.0f;

	/// @brief Number of samples stored in timeline history.
	int HistorySize = 240;

	/// @brief Last selected detail panel index.
	int ActivePanel = 0;

	/// @brief Whether live sampling starts paused.
	bool StartPaused = false;

	/// @brief Per-section visibility toggles.
	bool ShowPerformance = true;
	bool ShowRenderer = true;
	bool ShowScene = true;
	bool ShowLighting = true;
	bool ShowMemory = true;
	bool ShowAudio = true;
	bool ShowInput = true;
	bool ShowSystem = true;
};

/**
 * Represents the editor settings for the application, including appearance settings and auto-save interval.
 * 
 * The editor settings provide configuration options specific to the editor interface, allowing users to customize the appearance and behavior of the editor according to their preferences.
 */
struct EditorSettings {
	/// @brief The appearance settings that allow users to customize the visual aspects of the editor interface, such as the theme, font size, UI scale, and column width.
	AppearanceSettings Appearance;

	/// @brief The live scene camera settings used by the Scene window.
	SceneCameraSettings SceneCamera;

	/// @brief The default visibility of editor windows on startup.
	WindowsSettings Windows;

	/// @brief Preferences for the Statistics window UI and sampling behavior.
	StatisticsWindowSettings Statistics;

	/// @brief The interval in minutes for the auto-save feature, which determines how frequently the editor automatically saves the user's work to prevent data loss.
	int AutoSaveInterval = 10;
};

/**
 * Represents settings that control the interactive navigation of the Scene window.
 */
struct NavigationSettings {
	/// @brief Speed multiplier for panning/movement. Higher values move the view faster.
	float MovementSpeed = 2.0f;

	/// @brief Speed multiplier for fast panning/movement (Shift key). Higher values move the view faster.
	float FastMovementSpeed = 5.0f;

	/// @brief Speed multiplier for rotation (Q/E keys). Higher values rotate faster.
	float RotationSpeed = 2.0f;

	/// @brief Speed multiplier for zooming (scroll wheel / Shift / Ctrl). Higher values zoom faster.
	float ZoomSpeed = 2.0f;

	/// @brief Smoothing factor for parameter interpolation. Higher values produce snappier, faster transitions.
	float Smoothing = 5.0f;

	/// @brief When enabled, the scroll wheel zoom direction is reversed.
	bool InvertZoom = false;
};

/**
 * Represents settings that control how frames and configurations are exported.
 */
struct ExportSettings {
	/// @brief The image format used when exporting a frame (PNG, JPEG, or BMP).
	ExportImageFormat ImageFormat = ExportImageFormat::PNG;

	/// @brief JPEG compression quality in the range [0, 100]. Only relevant when `ImageFormat` is JPEG.
	int ImageQuality = 100;

	/// @brief Root folder where exported images and configurations are placed.
	std::filesystem::path Folder = "Export";
};

/**
 * Represents the overall settings for the application, including application settings, rendering settings, and editor settings.
 * 
 * The settings struct serves as a container for all the different categories of settings, allowing for organized management and easy access to various configuration options throughout the application.
 */
struct Settings {
	/// @brief The application settings that provide general configuration options for the application, such as its name, version, and behavior on startup.
	ApplicationSettings Application;

	/// @brief The audio settings that provide configuration options for audio input/output devices, volume levels, and other audio-related settings.
	AudioSettings Audio;

	/// @brief The rendering settings that determine how the application renders graphics, including the rendering engine, resolution, window mode, and VSync.
	RenderingSettings Rendering;

	/// @brief The editor settings that provide configuration options specific to the editor interface, allowing users to customize the appearance and behavior of the editor.
	EditorSettings Editor;

	/// @brief The navigation settings that control Scene window interaction speeds and behaviour.
	NavigationSettings Navigation;

	/// @brief The export settings that control the format and destination of exported images and configurations.
	ExportSettings Export;
};
#pragma once

#include "Core/Audio/AudioChannel.h"
#include "Core/Settings/Settings.h"

#include "ECS/Components/LightComponent.h"

#include "Editor/Theme/Theme.h"

#include <string>

class Utilities {
public:
	static std::string RenderingEngineToString(const RenderingEngine& engine);
	static RenderingEngine StringToRenderingEngine(const std::string& engine);

	static std::string EditorThemeToString(const EditorTheme& theme);
	static EditorTheme StringToEditorTheme(const std::string& theme);

	static std::string CategoryToString(const Category& category);
	static Category StringToCategory(const std::string& category);

	static std::string DirectionToString(const Direction& direction);
	static Direction StringToDirection(const std::string& direction);

	static std::string WindowModeToString(const WindowMode& mode);
	static WindowMode StringToWindowMode(const std::string& mode);

	static std::string ExportImageFormatToString(const ExportImageFormat& format);
	static ExportImageFormat StringToExportImageFormat(const std::string& format);

	static std::string LightTypeToString(const LightType& type);
	static LightType StringToLightType(const std::string& type);

	static std::string SampleRateToString(const SampleRate& sampleRate);
	static SampleRate StringToSampleRate(const std::string& sampleRate);

	static std::string BufferSizeToString(const BufferSize& bufferSize);
	static BufferSize StringToBufferSize(const std::string& bufferSize);

	static std::string AudioChannelToString(const AudioChannel& channel);
	static AudioChannel StringToAudioChannel(const std::string& channel);
};
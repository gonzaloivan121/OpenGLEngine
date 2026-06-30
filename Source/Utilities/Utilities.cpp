#include "Utilities.h"

#include "Core/Log/Log.h"

std::string Utilities::RenderingEngineToString(const RenderingEngine& engine) {
	switch (engine) {
		case RenderingEngine::OpenGL:	return "OpenGL";
		case RenderingEngine::DirectX:	return "DirectX";
		case RenderingEngine::Vulkan:	return "Vulkan";
		default:						return "Unknown";
	}
}

RenderingEngine Utilities::StringToRenderingEngine(const std::string& engine) {
	if (engine == "OpenGL")		return RenderingEngine::OpenGL;
	if (engine == "DirectX")	return RenderingEngine::DirectX;
	if (engine == "Vulkan")		return RenderingEngine::Vulkan;

	Log::Error("Utilities::StringToRenderingEngine - Unknown Rendering Engine");

	return RenderingEngine::OpenGL;
}

std::string Utilities::EditorThemeToString(const EditorTheme& theme) {
	switch (theme) {
		case EditorTheme::AdobeInspired:			return "Adobe Inspired";
		case EditorTheme::AyuDark:					return "Ayu Dark";
		case EditorTheme::BessDark:					return "Bess Dark";
		case EditorTheme::BlackDevil:				return "Black Devil";
		case EditorTheme::BootstrapDark:			return "Bootstrap Dark";
		case EditorTheme::Carbon:					return "Carbon";
		case EditorTheme::Cherno:					return "Cherno";
		case EditorTheme::Cherry:					return "Cherry";
		case EditorTheme::ClassicSteam:				return "Classic Steam";
		case EditorTheme::Classic:					return "Classic";
		case EditorTheme::ComfortableDarkCyan:		return "Comfortable Dark Cyan";
		case EditorTheme::ComfortableLightOrange:	return "Comfortable Light Orange";
		case EditorTheme::Comfy:					return "Comfy";
		case EditorTheme::Darcula:					return "Darcula";
		case EditorTheme::DarkRed:					return "Dark Red";
		case EditorTheme::DarkRuda:					return "Dark Ruda";
		case EditorTheme::Dark:						return "Dark";
		case EditorTheme::Darky:					return "Darky";
		case EditorTheme::DeepDark:					return "Deep Dark";
		case EditorTheme::DiscordDark:				return "Discord Dark";
		case EditorTheme::Enemymouse:				return "Enemymouse";
		case EditorTheme::Everforest:				return "Everforest";
		case EditorTheme::Excellency:				return "Excellency";
		case EditorTheme::FutureDark:				return "Future Dark";
		case EditorTheme::Gold:						return "Gold";
		case EditorTheme::GreenFont:				return "Green Font";
		case EditorTheme::GreenLeaf:				return "Green Leaf";
		case EditorTheme::HazyDark:					return "Hazy Dark";
		case EditorTheme::LedSynthmaster:			return "Led Synthmaster";
		case EditorTheme::Light:					return "Light";
		case EditorTheme::MaterialFlat:				return "Material Flat";
		case EditorTheme::Microsoft:				return "Microsoft";
		case EditorTheme::Modern:					return "Modern";
		case EditorTheme::Photoshop:				return "Photoshop";
		case EditorTheme::PurpleComfy:				return "Purple Comfy";
		case EditorTheme::QuickMinimalLook:			return "Quick Minimal Look";
		case EditorTheme::RedFont:					return "Red Font";
		case EditorTheme::RedOni:					return "Red Oni";
		case EditorTheme::Rest:						return "Rest";
		case EditorTheme::RoundedVisualStudio:		return "Rounded Visual Studio";
		case EditorTheme::SoftCherry:				return "Soft Cherry";
		case EditorTheme::SonicRiders:				return "Sonic Riders";
		case EditorTheme::Unreal:					return "Unreal";
		case EditorTheme::VisualStudio:				return "Visual Studio";
		case EditorTheme::Windark:					return "Windark";
		default:									return "Unknown";
	}
}

EditorTheme Utilities::StringToEditorTheme(const std::string& theme) {
	if (theme == "Adobe Inspired")				return EditorTheme::AdobeInspired;
	if (theme == "Ayu Dark")					return EditorTheme::AyuDark;
	if (theme == "Bess Dark")					return EditorTheme::BessDark;
	if (theme == "Black Devil")					return EditorTheme::BlackDevil;
	if (theme == "Bootstrap Dark")				return EditorTheme::BootstrapDark;
	if (theme == "Carbon")						return EditorTheme::Carbon;
	if (theme == "Cherno")						return EditorTheme::Cherno;
	if (theme == "Cherry")						return EditorTheme::Cherry;
	if (theme == "Classic Steam")				return EditorTheme::ClassicSteam;
	if (theme == "Classic")						return EditorTheme::Classic;
	if (theme == "Comfortable Dark Cyan")		return EditorTheme::ComfortableDarkCyan;
	if (theme == "Comfortable Light Orange")	return EditorTheme::ComfortableLightOrange;
	if (theme == "Comfy")						return EditorTheme::Comfy;
	if (theme == "Darcula")						return EditorTheme::Darcula;
	if (theme == "Dark Red")					return EditorTheme::DarkRed;
	if (theme == "Dark Ruda")					return EditorTheme::DarkRuda;
	if (theme == "Dark")						return EditorTheme::Dark;
	if (theme == "Darky")						return EditorTheme::Darky;
	if (theme == "Deep Dark")					return EditorTheme::DeepDark;
	if (theme == "Discord Dark")				return EditorTheme::DiscordDark;
	if (theme == "Enemymouse")					return EditorTheme::Enemymouse;
	if (theme == "Everforest")					return EditorTheme::Everforest;
	if (theme == "Excellency")					return EditorTheme::Excellency;
	if (theme == "Future Dark")					return EditorTheme::FutureDark;
	if (theme == "Gold")						return EditorTheme::Gold;
	if (theme == "Green Font")					return EditorTheme::GreenFont;
	if (theme == "Green Leaf")					return EditorTheme::GreenLeaf;
	if (theme == "Hazy Dark")					return EditorTheme::HazyDark;
	if (theme == "Led Synthmaster")				return EditorTheme::LedSynthmaster;
	if (theme == "Light")						return EditorTheme::Light;
	if (theme == "Material Flat")				return EditorTheme::MaterialFlat;
	if (theme == "Microsoft")					return EditorTheme::Microsoft;
	if (theme == "Modern")						return EditorTheme::Modern;
	if (theme == "Photoshop")					return EditorTheme::Photoshop;
	if (theme == "Purple Comfy")				return EditorTheme::PurpleComfy;
	if (theme == "Quick Minimal Look")			return EditorTheme::QuickMinimalLook;
	if (theme == "Red Font")					return EditorTheme::RedFont;
	if (theme == "Red Oni")						return EditorTheme::RedOni;
	if (theme == "Rest")						return EditorTheme::Rest;
	if (theme == "Rounded Visual Studio")		return EditorTheme::RoundedVisualStudio;
	if (theme == "Soft Cherry")					return EditorTheme::SoftCherry;
	if (theme == "Sonic Riders")				return EditorTheme::SonicRiders;
	if (theme == "Unreal")						return EditorTheme::Unreal;
	if (theme == "Visual Studio")				return EditorTheme::VisualStudio;
	if (theme == "Windark")						return EditorTheme::Windark;

	Log::Error("Utilities::StringToEditorTheme - Unknown Editor Theme");

	return EditorTheme::Excellency;
}

std::string Utilities::CategoryToString(const Category& category) {
	switch (category) {
		case Category::Application:		return "Application";
		case Category::Audio:			return "Audio";
		case Category::Editor:			return "Editor";
		case Category::Graphics:		return "Graphics";
		case Category::Input:			return "Input";
		case Category::Localization:	return "Localization";
		case Category::Quality:			return "Quality";
		case Category::Rendering:		return "Rendering";
		case Category::Time:			return "Time";
		case Category::Export:			return "Export";
		default:						return "Unknown";
	}
}

Category Utilities::StringToCategory(const std::string& category) {
	if (category == "Application")	return Category::Application;
	if (category == "Audio")		return Category::Audio;
	if (category == "Editor")		return Category::Editor;
	if (category == "Graphics")		return Category::Graphics;
	if (category == "Input")		return Category::Input;
	if (category == "Localization")	return Category::Localization;
	if (category == "Quality")		return Category::Quality;
	if (category == "Rendering")	return Category::Rendering;
	if (category == "Time")			return Category::Time;
	if (category == "Export")		return Category::Export;

	Log::Error("Utilities::StringToCategory - Unknown Category");

	return Category::Application;
}

std::string Utilities::DirectionToString(const Direction& direction) {
	switch (direction) {
		case Direction::None:	return "None";
		case Direction::Left:	return "Left";
		case Direction::Right:	return "Right";
		case Direction::Up:		return "Up";
		case Direction::Down:	return "Down";
		default:				return "Unknown";
	}
}

Direction Utilities::StringToDirection(const std::string& direction) {
	if (direction == "None")	return Direction::None;
	if (direction == "Left")	return Direction::Left;
	if (direction == "Right")	return Direction::Right;
	if (direction == "Up")		return Direction::Up;
	if (direction == "Down")	return Direction::Down;

	Log::Error("Utilities::StringToDirection - Unknown Direction");

	return Direction::Left;
}

std::string Utilities::WindowModeToString(const WindowMode& mode) {
	switch (mode) {
		case WindowMode::Windowed:		return "Windowed";
		case WindowMode::Fullscreen:	return "Fullscreen";
		case WindowMode::Borderless:	return "Borderless";
		default:						return "Unknown";
	}
}

WindowMode Utilities::StringToWindowMode(const std::string& mode) {
	if (mode == "Windowed")		return WindowMode::Windowed;
	if (mode == "Fullscreen")	return WindowMode::Fullscreen;
	if (mode == "Borderless")	return WindowMode::Borderless;

	Log::Error("Utilities::StringToWindowMode - Unknown Window Mode");

	return WindowMode::Windowed;
}

std::string Utilities::ExportImageFormatToString(const ExportImageFormat& format) {
	switch (format) {
		case ExportImageFormat::PNG:	return "PNG";
		case ExportImageFormat::JPEG:	return "JPEG";
		case ExportImageFormat::BMP:	return "BMP";
		default:						return "Unknown";
	}
}

ExportImageFormat Utilities::StringToExportImageFormat(const std::string& format) {
	if (format == "PNG")	return ExportImageFormat::PNG;
	if (format == "JPEG")	return ExportImageFormat::JPEG;
	if (format == "BMP")	return ExportImageFormat::BMP;

	Log::Error("Utilities::StringToExportImageFormat - Unknown Export Image Format");

	return ExportImageFormat::PNG;
}

std::string Utilities::LightTypeToString(const LightType& type) {
	switch (type) {
		case LightType::Directional: return "Directional";
		case LightType::Point:       return "Point";
		case LightType::Spot:        return "Spot";
		default:                     return "Unknown";
	}
}

LightType Utilities::StringToLightType(const std::string& type) {
	if (type == "Directional")	return LightType::Directional;
	if (type == "Point")		return LightType::Point;
	if (type == "Spot")			return LightType::Spot;

	Log::Error("Utilities::StringToLightType - Unknown Light Type");

	return LightType::Directional;
}

std::string Utilities::SampleRateToString(const SampleRate& sampleRate) {
	switch (sampleRate) {
		case SampleRate::SR_44100:	return "44100";
		case SampleRate::SR_48000:	return "48000";
		case SampleRate::SR_88200:	return "88200";
		case SampleRate::SR_96000:	return "96000";
		case SampleRate::SR_176400:	return "176400";
		case SampleRate::SR_192000:	return "192000";
		default:					return "Unknown";
	}
}

SampleRate Utilities::StringToSampleRate(const std::string& sampleRate) {
	if (sampleRate == "44100")	return SampleRate::SR_44100;
	if (sampleRate == "48000")	return SampleRate::SR_48000;
	if (sampleRate == "88200")	return SampleRate::SR_88200;
	if (sampleRate == "96000")	return SampleRate::SR_96000;
	if (sampleRate == "176400")	return SampleRate::SR_176400;
	if (sampleRate == "192000")	return SampleRate::SR_192000;

	Log::Error("Utilities::StringToSampleRate - Unknown Sample Rate");

	return SampleRate::SR_44100;
}

std::string Utilities::BufferSizeToString(const BufferSize& bufferSize) {
	switch (bufferSize) {
		case BufferSize::BS_16:		return "16";
		case BufferSize::BS_32:		return "32";
		case BufferSize::BS_48:		return "48";
		case BufferSize::BS_64:		return "64";
		case BufferSize::BS_96:		return "96";
		case BufferSize::BS_128:	return "128";
		case BufferSize::BS_160:	return "160";
		case BufferSize::BS_192:	return "192";
		case BufferSize::BS_256:	return "256";
		case BufferSize::BS_512:	return "512";
		case BufferSize::BS_1024:	return "1024";
		default:					return "Unknown";
	}
}

BufferSize Utilities::StringToBufferSize(const std::string& bufferSize) {
	if (bufferSize == "16")		return BufferSize::BS_16;
	if (bufferSize == "32")		return BufferSize::BS_32;
	if (bufferSize == "48")		return BufferSize::BS_48;
	if (bufferSize == "64")		return BufferSize::BS_64;
	if (bufferSize == "96")		return BufferSize::BS_96;
	if (bufferSize == "128")	return BufferSize::BS_128;
	if (bufferSize == "160")	return BufferSize::BS_160;
	if (bufferSize == "192")	return BufferSize::BS_192;
	if (bufferSize == "256")	return BufferSize::BS_256;
	if (bufferSize == "512")	return BufferSize::BS_512;
	if (bufferSize == "1024")	return BufferSize::BS_1024;

	Log::Error("Utilities::StringToBufferSize - Unknown Buffer Size");

	return BufferSize::BS_16;
}

std::string Utilities::AudioChannelToString(const AudioChannel& channel) {
	switch (channel) {
		case AudioChannel::Ambience:	return "Ambience";
		case AudioChannel::Music:		return "Music";
		case AudioChannel::Effects:		return "Effects";
		case AudioChannel::Voices:		return "Voices";
		default:						return "Unknown";
	}
}

AudioChannel Utilities::StringToAudioChannel(const std::string& channel) {
	if (channel == "Ambience")	return AudioChannel::Ambience;
	if (channel == "Music")		return AudioChannel::Music;
	if (channel == "Effects")	return AudioChannel::Effects;
	if (channel == "Voices")	return AudioChannel::Voices;

	Log::Error("Utilities::StringToAudioChannel - Unknown Audio Channel");

	return AudioChannel::Ambience;
}

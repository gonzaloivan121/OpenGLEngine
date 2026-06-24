#pragma once

namespace Payload {
	enum class Type {
		Folder,
		Scene,
		Material,
		Texture,
		Mesh,
		Audio,
		Shader,
	};

	constexpr const char* ToUIPayload(Type type) {
		switch (type) {
			case Type::Folder:   return "PAYLOAD_FOLDER";
			case Type::Scene:    return "PAYLOAD_SCENE";
			case Type::Material: return "PAYLOAD_MATERIAL";
			case Type::Texture:  return "PAYLOAD_TEXTURE";
			case Type::Mesh:     return "PAYLOAD_MESH";
			case Type::Audio:    return "PAYLOAD_AUDIO";
			case Type::Shader:   return "PAYLOAD_SHADER";
		}

		return "PAYLOAD_UNKNOWN";
	}
}
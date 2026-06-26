#pragma once

#include "ECS/ShaderAsset.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

class ShaderSerializer {
public:
	ShaderSerializer(ShaderAsset& shader);

	bool Serialize(const std::filesystem::path& filepath);
	bool Deserialize(const std::filesystem::path& filepath);

private:
	ShaderAsset& m_Shader;
};
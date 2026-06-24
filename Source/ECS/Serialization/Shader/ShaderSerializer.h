#pragma once

#include "ECS/Shader.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

class ShaderSerializer {
public:
	ShaderSerializer(Shader& shader);

	bool Serialize(const std::filesystem::path& filepath);
	bool Deserialize(const std::filesystem::path& filepath);

private:
	Shader& m_Shader;
};
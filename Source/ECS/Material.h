#pragma once

#include <glm/glm.hpp>
#include <filesystem>

struct Emission {
	bool Enabled = false;
	glm::vec4 Color = glm::vec4(1.0f);
	float Intensity = 1.0f;
};

struct Textures {
	glm::vec2 Tiling = glm::vec2(1.0f, 1.0f);
	glm::vec2 Offset = glm::vec2(0.0f, 0.0f);
	std::filesystem::path AlbedoFilepath;
	std::filesystem::path NormalFilepath;
	std::filesystem::path MetallicFilepath;
	std::filesystem::path RoughnessFilepath;
	std::filesystem::path AmbientOcclusionFilepath;
	std::filesystem::path HeightFilepath;
	std::filesystem::path EmissionFilepath;
};

struct Material {
	std::filesystem::path ShaderFilepath;
	glm::vec4 Albedo = glm::vec4(1.0f);
	float Metallic = 0.5f;
	float Roughness = 0.5f;
	float AmbientOcclusion = 1.0f;
	float HeightScale = 0.1f;
	Emission Emission;
	Textures Textures;
};
#pragma once

#include "ECS/Material.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

class MaterialSerializer {
public:
	MaterialSerializer(Material& material);

	bool Serialize(const std::filesystem::path& filepath);
	bool Deserialize(const std::filesystem::path& filepath);

private:
	void SerializeMaterial(YAML::Emitter& out, const Material& material);

	void SerializeEmission(YAML::Emitter& out, const Emission& emission);
	void SerializeTextures(YAML::Emitter& out, const Textures& textures);

private:
	void DeserializeMaterial(const YAML::Node& materialNode, Material& material);

	void DeserializeEmission(const YAML::Node& emissionNode, Emission& emission);
	void DeserializeTextures(const YAML::Node& texturesNode, Textures& textures);

private:
	Material& m_Material;
};
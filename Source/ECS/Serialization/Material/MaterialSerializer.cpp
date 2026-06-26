#include "MaterialSerializer.h"

#include "Core/Log/Log.h"

#include "Utilities/Utilities.h"

#include <fstream>

namespace YAML {
	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	static Emitter& operator<<(Emitter& out, const glm::vec3& v) {
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << EndSeq;
		return out;
	}

	static Emitter& operator<<(Emitter& out, const glm::vec2& v) {
		out << Flow;
		out << BeginSeq << v.x << v.y << EndSeq;
		return out;
	}
}

MaterialSerializer::MaterialSerializer(Material& material)
	: m_Material(material)
{
	Log::Trace("MaterialSerializer::MaterialSerializer - Setting up Material Serializer");
}

bool MaterialSerializer::Serialize(const std::filesystem::path& filepath) {
	Log::Trace("MaterialSerializer::Serialize - Saving Material: " + filepath.string());

	if (filepath.empty()) {
		Log::Error("MaterialSerializer::Serialize - Filepath is empty");
		return false;
	}

	Log::Trace("MaterialSerializer::Serialize - Serializing Material into file: " + filepath.string());

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		{
			SerializeMaterial(out, m_Material);
		}
		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);

	if (fout.bad()) {
		Log::Error("MaterialSerializer::Serialize - Error during Material Serialization: Bad file");
		return false;
	}

	if (!fout.is_open()) {
		Log::Error("MaterialSerializer::Serialize - Error during Material Serialization: File not open");
		return false;
	}

	if (!fout.good()) {
		Log::Error("MaterialSerializer::Serialize - Error during Material Serialization: File not good");
		return false;
	}

	if (!out.good()) {
		Log::Error("MaterialSerializer::Serialize - Error during Material Serialization: YAML Emitter not good");
		return false;
	}

	fout << out.c_str();

	Log::Trace("MaterialSerializer::Serialize - Material Serialization complete");

	return true;
}

bool MaterialSerializer::Deserialize(const std::filesystem::path& filepath) {
	YAML::Node data;

	try {
		Log::Trace("MaterialSerializer::Deserialize - Attempting to read Material file: " + filepath.string());

		// Load the YAML data from the specified file path. If the file cannot be read or parsed, an exception will be thrown.
		data = YAML::LoadFile(filepath.string());
	} catch (YAML::Exception e) {
		Log::Error("MaterialSerializer::Deserialize - Error reading Material file: " + e.msg);

		// Return false to indicate failure due to an error while reading the file.
		return false;
	}

	const auto& materialNode = data["Material"];

	if (!materialNode) {
		Log::Error("MaterialSerializer::Deserialize - Error loading Material file: " + filepath.string());
		return false;
	}

	Log::Trace("MaterialSerializer::Deserialize - Loading Material from file: " + filepath.string());

	DeserializeMaterial(materialNode, m_Material);

	Log::Trace("MaterialSerializer::Deserialize - Loading complete");

	return true;
}

void MaterialSerializer::SerializeMaterial(YAML::Emitter& out, const Material& material) {
	out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap; // Material
	{
		out << YAML::Key << "ShaderFilepath" << YAML::Value << material.ShaderFilepath.string();
		out << YAML::Key << "Albedo" << YAML::Value << material.Albedo;
		out << YAML::Key << "Metallic" << YAML::Value << material.Metallic;
		out << YAML::Key << "Roughness" << YAML::Value << material.Roughness;
		out << YAML::Key << "AmbientOcclusion" << YAML::Value << material.AmbientOcclusion;
		out << YAML::Key << "HeightScale" << YAML::Value << material.HeightScale;

		SerializeEmission(out, material.Emission);
		SerializeTextures(out, material.Textures);
	}
	out << YAML::EndMap; // Material
}

void MaterialSerializer::SerializeEmission(YAML::Emitter& out, const Emission& emission) {
	out << YAML::Key << "Emission" << YAML::Value << YAML::BeginMap; // Emission
	{
		out << YAML::Key << "Enabled" << YAML::Value << emission.Enabled;
		out << YAML::Key << "Color" << YAML::Value << emission.Color;
		out << YAML::Key << "Intensity" << YAML::Value << emission.Intensity;
	}
	out << YAML::EndMap; // Emission
}

void MaterialSerializer::SerializeTextures(YAML::Emitter& out, const Textures& textures) {
	out << YAML::Key << "Textures" << YAML::Value << YAML::BeginMap; // Textures
	{
		out << YAML::Key << "Tiling" << YAML::Value << textures.Tiling;
		out << YAML::Key << "Offset" << YAML::Value << textures.Offset;
		out << YAML::Key << "AlbedoFilepath" << YAML::Value << textures.AlbedoFilepath.string();
		out << YAML::Key << "NormalFilepath" << YAML::Value << textures.NormalFilepath.string();
		out << YAML::Key << "MetallicFilepath" << YAML::Value << textures.MetallicFilepath.string();
		out << YAML::Key << "RoughnessFilepath" << YAML::Value << textures.RoughnessFilepath.string();
		out << YAML::Key << "AmbientOcclusionFilepath" << YAML::Value << textures.AmbientOcclusionFilepath.string();
		out << YAML::Key << "HeightFilepath" << YAML::Value << textures.HeightFilepath.string();
		out << YAML::Key << "EmissionFilepath" << YAML::Value << textures.EmissionFilepath.string();
	}
	out << YAML::EndMap; // Textures
}

void MaterialSerializer::DeserializeMaterial(const YAML::Node& materialNode, Material& material) {
	if (const auto& shaderFilepathNode = materialNode["ShaderFilepath"]) {
		material.ShaderFilepath = shaderFilepathNode.as<std::string>();
	}

	if (const auto& albedoNode = materialNode["Albedo"]) {
		material.Albedo = albedoNode.as<glm::vec3>();
	}

	if (const auto& metallicNode = materialNode["Metallic"]) {
		material.Metallic = metallicNode.as<float>();
	}

	if (const auto& roughnessNode = materialNode["Roughness"]) {
		material.Roughness = roughnessNode.as<float>();
	}

	if (const auto& ambientOcclusionNode = materialNode["AmbientOcclusion"]) {
		material.AmbientOcclusion = ambientOcclusionNode.as<float>();
	}

	if (const auto& heightScaleNode = materialNode["HeightScale"]) {
		material.HeightScale = heightScaleNode.as<float>();
	}

	if (const auto& emissionNode = materialNode["Emission"]) {
		DeserializeEmission(emissionNode, material.Emission);
	}

	if (const auto& texturesNode = materialNode["Textures"]) {
		DeserializeTextures(texturesNode, material.Textures);
	}
}

void MaterialSerializer::DeserializeEmission(const YAML::Node& emissionNode, Emission& emission) {
	if (const auto& enabledNode = emissionNode["Enabled"]) {
		emission.Enabled = enabledNode.as<bool>();
	}

	if (const auto& colorNode = emissionNode["Color"]) {
		emission.Color = colorNode.as<glm::vec3>();
	}

	if (const auto& intensityNode = emissionNode["Intensity"]) {
		emission.Intensity = intensityNode.as<float>();
	}
}

void MaterialSerializer::DeserializeTextures(const YAML::Node& texturesNode, Textures& textures) {
	if (const auto& tilingNode = texturesNode["Tiling"]) {
		textures.Tiling = tilingNode.as<glm::vec2>();
	}

	if (const auto& offsetNode = texturesNode["Offset"]) {
		textures.Offset = offsetNode.as<glm::vec2>();
	}

	if (const auto& albedoFilepathNode = texturesNode["AlbedoFilepath"]) {
		textures.AlbedoFilepath = albedoFilepathNode.as<std::string>();
	}

	if (const auto& normalFilepathNode = texturesNode["NormalFilepath"]) {
		textures.NormalFilepath = normalFilepathNode.as<std::string>();
	}

	if (const auto& metallicFilepathNode = texturesNode["MetallicFilepath"]) {
		textures.MetallicFilepath = metallicFilepathNode.as<std::string>();
	}

	if (const auto& roughnessFilepathNode = texturesNode["RoughnessFilepath"]) {
		textures.RoughnessFilepath = roughnessFilepathNode.as<std::string>();
	}

	if (const auto& ambientOcclusionFilepathNode = texturesNode["AmbientOcclusionFilepath"]) {
		textures.AmbientOcclusionFilepath = ambientOcclusionFilepathNode.as<std::string>();
	}

	if (const auto& heightFilepathNode = texturesNode["HeightFilepath"]) {
		textures.HeightFilepath = heightFilepathNode.as<std::string>();
	}

	if (const auto& emissionFilepathNode = texturesNode["EmissionFilepath"]) {
		textures.EmissionFilepath = emissionFilepathNode.as<std::string>();
	}
}

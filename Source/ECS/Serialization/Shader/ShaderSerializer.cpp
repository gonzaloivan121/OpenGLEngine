#include "ShaderSerializer.h"

#include "Core/Log/Log.h"

#include <fstream>

ShaderSerializer::ShaderSerializer(Shader& shader)
	: m_Shader(shader)
{
	Log::Trace("ShaderSerializer::ShaderSerializer - Setting up Shader Serializer");
}

bool ShaderSerializer::Serialize(const std::filesystem::path& filepath) {
	Log::Trace("ShaderSerializer::Serialize - Saving Shader: " + filepath.string());

	if (filepath.empty()) {
		Log::Error("ShaderSerializer::Serialize - Filepath is empty");
		return false;
	}

	Log::Trace("ShaderSerializer::Serialize - Serializing Shader into file: " + filepath.string());

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		{
			out << YAML::Key << "Shader" << YAML::Value << YAML::BeginMap; // Shader
			{
				out << YAML::Key << "VertexFilepath" << YAML::Value << m_Shader.VertexFilepath.string();
				out << YAML::Key << "FragmentFilepath" << YAML::Value << m_Shader.FragmentFilepath.string();
			}
			out << YAML::EndMap; // Shader
		}
		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);

	if (fout.bad()) {
		Log::Error("ShaderSerializer::Serialize - Error during Shader Serialization: Bad file");
		return false;
	}

	if (!fout.is_open()) {
		Log::Error("ShaderSerializer::Serialize - Error during Shader Serialization: File not open");
		return false;
	}

	if (!fout.good()) {
		Log::Error("ShaderSerializer::Serialize - Error during Shader Serialization: File not good");
		return false;
	}

	if (!out.good()) {
		Log::Error("ShaderSerializer::Serialize - Error during Shader Serialization: YAML Emitter not good");
		return false;
	}

	fout << out.c_str();

	Log::Trace("ShaderSerializer::Serialize - Shader Serialization complete");

	return true;
}

bool ShaderSerializer::Deserialize(const std::filesystem::path& filepath) {
	YAML::Node data;

	try {
		Log::Trace("ShaderSerializer::Deserialize - Attempting to read Shader file: " + filepath.string());

		// Load the YAML data from the specified file path. If the file cannot be read or parsed, an exception will be thrown.
		data = YAML::LoadFile(filepath.string());
	} catch (YAML::Exception e) {
		Log::Error("ShaderSerializer::Deserialize - Error reading Shader file: " + e.msg);

		// Return false to indicate failure due to an error while reading the file.
		return false;
	}

	const auto& shaderNode = data["Shader"];

	if (!shaderNode) {
		Log::Error("ShaderSerializer::Deserialize - Error loading Shader file: " + filepath.string());
		return false;
	}

	Log::Trace("ShaderSerializer::Deserialize - Loading Shader from file: " + filepath.string());

	if (const auto& vertexFilepathNode = shaderNode["VertexFilepath"]) {
		m_Shader.VertexFilepath = vertexFilepathNode.as<std::string>();
	}

	if (const auto& fragmentFilepathNode = shaderNode["FragmentFilepath"]) {
		m_Shader.FragmentFilepath = fragmentFilepathNode.as<std::string>();
	}

	Log::Trace("ShaderSerializer::Deserialize - Loading complete");

	return true;
}

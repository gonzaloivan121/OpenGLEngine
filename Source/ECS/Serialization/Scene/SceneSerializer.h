#pragma once

#include "ECS/Scene.h"
#include "ECS/Components.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

class SceneSerializer {
public:
	SceneSerializer(Scene& scene);

	bool Serialize(const std::filesystem::path& filepath);
	bool Deserialize(const std::filesystem::path& filepath);

private:
	void SerializeEntity(YAML::Emitter& out, const Entity& entity);

	void SerializeCameraComponent(YAML::Emitter& out, const CameraComponent* camera);
	void SerializeLightComponent(YAML::Emitter& out, const LightComponent* light);
	void SerializeMaterialComponent(YAML::Emitter& out, const MaterialComponent* material);
	void SerializeMeshComponent(YAML::Emitter& out, const MeshComponent* mesh);
	void SerializeNameComponent(YAML::Emitter& out, const NameComponent* name);
	void SerializeShaderComponent(YAML::Emitter& out, const ShaderComponent* shader);
	void SerializeTransformComponent(YAML::Emitter& out, const TransformComponent* transform);

private:
	void DeserializeEntity(const YAML::Node& entityNode);

	void DeserializeCameraComponent(const YAML::Node& cameraNode, Entity& entity);
	void DeserializeLightComponent(const YAML::Node& lightNode, Entity& entity);
	void DeserializeMaterialComponent(const YAML::Node& materialNode, Entity& entity);
	void DeserializeMeshComponent(const YAML::Node& meshNode, Entity& entity);
	void DeserializeNameComponent(const YAML::Node& nameNode, Entity& entity);
	void DeserializeShaderComponent(const YAML::Node& shaderNode, Entity& entity);
	void DeserializeTransformComponent(const YAML::Node& transformNode, Entity& entity);

private:
	Scene& m_Scene;
};
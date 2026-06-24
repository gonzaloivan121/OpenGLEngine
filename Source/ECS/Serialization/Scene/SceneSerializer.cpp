#include "SceneSerializer.h"

#include "Core/Log/Log.h"

#include "ECS/Serialization/Material/MaterialSerializer.h"

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

SceneSerializer::SceneSerializer(Scene& scene)
	: m_Scene(scene)
{
	Log::Trace("SceneSerializer::SceneSerializer - Setting up Scene Serializer");
}

bool SceneSerializer::Serialize(const std::filesystem::path& filepath) {
	Log::Trace("SceneSerializer::Serialize - Saving Scene: " + filepath.string());

	if (filepath.empty()) {
		Log::Error("SceneSerializer::Serialize - Filepath is empty");
		return false;
	}

	Log::Trace("SceneSerializer::Serialize - Serializing Scene into file: " + filepath.string());

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		{
			out << YAML::Key << "Scene" << YAML::Value << YAML::BeginMap; // Scene
			{
				out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq; // Entities
				{
					auto& entities = m_Scene.GetEntityIDs();

					for (auto& entity : entities) {
						SerializeEntity(out, m_Scene.GetEntity(entity));
					}

				}
				out << YAML::EndSeq; // Entities
			}
			out << YAML::EndMap; // Scene
		}
		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);

	if (fout.bad()) {
		Log::Error("SceneSerializer::Serialize - Error during Scene Serialization: Bad file");
		return false;
	}

	if (!fout.is_open()) {
		Log::Error("SceneSerializer::Serialize - Error during Scene Serialization: File not open");
		return false;
	}

	if (!fout.good()) {
		Log::Error("SceneSerializer::Serialize - Error during Scene Serialization: File not good");
		return false;
	}

	if (!out.good()) {
		Log::Error("SceneSerializer::Serialize - Error during Scene Serialization: YAML Emitter not good");
		return false;
	}

	fout << out.c_str();

	Log::Trace("SceneSerializer::Serialize - Scene Serialization complete");

	return true;
}

bool SceneSerializer::Deserialize(const std::filesystem::path& filepath) {
	YAML::Node data;

	try {
		Log::Trace("SceneSerializer::Deserialize - Attempting to read Scene file: " + filepath.string());

		// Load the YAML data from the specified file path. If the file cannot be read or parsed, an exception will be thrown.
		data = YAML::LoadFile(filepath.string());
	} catch (YAML::Exception e) {
		Log::Error("SceneSerializer::Deserialize - Error reading Scene file: " + e.msg);

		// Return false to indicate failure due to an error while reading the file.
		return false;
	}

	const auto& sceneNode = data["Scene"];

	if (!sceneNode) {
		Log::Error("SceneSerializer::Deserialize - Error loading Scene file: " + filepath.string());
		return false;
	}

	Log::Trace("SceneSerializer::Deserialize - Loading Scene from file: " + filepath.string());

	if (const auto& entitiesNode = sceneNode["Entities"]) {
		for (const auto& entityNode : entitiesNode) {
			const auto& wrappedEntityNode = entityNode["Entity"];
			DeserializeEntity(wrappedEntityNode ? wrappedEntityNode : entityNode);
		}
	}

	Log::Trace("SceneSerializer::Deserialize - Loading complete");

	return true;
}

void SceneSerializer::SerializeEntity(YAML::Emitter& out, const Entity& entity) {
	out << YAML::BeginMap; // Entity sequence item
	{
		out << YAML::Key << "Entity" << YAML::Value << YAML::BeginMap; // Entity
		{
			out << YAML::Key << "ID" << YAML::Value << static_cast<uint64_t>(entity.ID);
			out << YAML::Key << "Components" << YAML::Value << YAML::BeginMap; // Components

			{
				if (entity.HasComponent<NameComponent>()) {
					SerializeNameComponent(out, entity.GetComponent<NameComponent>());
				}

				if (entity.HasComponent<TransformComponent>()) {
					SerializeTransformComponent(out, entity.GetComponent<TransformComponent>());
				}

				if (entity.HasComponent<CameraComponent>()) {
					SerializeCameraComponent(out, entity.GetComponent<CameraComponent>());
				}

				if (entity.HasComponent<LightComponent>()) {
					SerializeLightComponent(out, entity.GetComponent<LightComponent>());
				}

				if (entity.HasComponent<MaterialComponent>()) {
					SerializeMaterialComponent(out, entity.GetComponent<MaterialComponent>());
				}

				if (entity.HasComponent<MeshComponent>()) {
					SerializeMeshComponent(out, entity.GetComponent<MeshComponent>());
				}

				if (entity.HasComponent<ShaderComponent>()) {
					SerializeShaderComponent(out, entity.GetComponent<ShaderComponent>());
				}
			}

			out << YAML::EndMap; // Components
		}
		out << YAML::EndMap; // Entity
	}
	out << YAML::EndMap; // Entity sequence item
}

void SceneSerializer::SerializeCameraComponent(YAML::Emitter& out, const CameraComponent* camera) {
	out << YAML::Key << "CameraComponent" << YAML::Value << YAML::BeginMap; // CameraComponent
	{
		out << YAML::Key << "FOV" << YAML::Value << camera->FOV;
		out << YAML::Key << "NearClip" << YAML::Value << camera->NearClip;
		out << YAML::Key << "FarClip" << YAML::Value << camera->FarClip;
		out << YAML::Key << "Primary" << YAML::Value << camera->Primary;
	}
	out << YAML::EndMap; // CameraComponent
}

void SceneSerializer::SerializeLightComponent(YAML::Emitter& out, const LightComponent* light) {
	out << YAML::Key << "LightComponent" << YAML::Value << YAML::BeginMap; // LightComponent
	{
		out << YAML::Key << "Type" << YAML::Value << Utilities::LightTypeToString(light->Type);
		out << YAML::Key << "Color" << YAML::Value << light->Color;
		out << YAML::Key << "Intensity" << YAML::Value << light->Intensity;
		out << YAML::Key << "Range" << YAML::Value << light->Range;

		if (light->Type == LightType::Directional) {
			out << YAML::Key << "Direction" << YAML::Value << light->Direction;
		} else if (light->Type == LightType::Point) {
			out << YAML::Key << "Constant" << YAML::Value << light->Constant;
			out << YAML::Key << "Linear" << YAML::Value << light->Linear;
			out << YAML::Key << "Quadratic" << YAML::Value << light->Quadratic;
		} else if (light->Type == LightType::Spot) {
			out << YAML::Key << "InnerCone" << YAML::Value << light->InnerCone;
			out << YAML::Key << "OuterCone" << YAML::Value << light->OuterCone;
		}
	}
	out << YAML::EndMap; // LightComponent
}

void SceneSerializer::SerializeMaterialComponent(YAML::Emitter& out, const MaterialComponent* material) {
	out << YAML::Key << "MaterialComponent" << YAML::Value << YAML::BeginMap; // MaterialComponent
	{
		out << YAML::Key << "MaterialFilepath" << YAML::Value << material->MaterialFilepath.string();
	}
	out << YAML::EndMap; // MaterialComponent
}

void SceneSerializer::SerializeMeshComponent(YAML::Emitter& out, const MeshComponent* mesh) {
	out << YAML::Key << "MeshComponent" << YAML::Value << YAML::BeginMap; // MeshComponent
	{
		out << YAML::Key << "MeshFilepath" << YAML::Value << mesh->MeshFilepath.string();
		out << YAML::Key << "CastShadows" << YAML::Value << mesh->CastShadows;
		out << YAML::Key << "ReceiveShadows" << YAML::Value << mesh->ReceiveShadows;
	}
	out << YAML::EndMap; // MeshComponent
}

void SceneSerializer::SerializeNameComponent(YAML::Emitter& out, const NameComponent* name) {
	out << YAML::Key << "NameComponent" << YAML::Value << YAML::BeginMap; // NameComponent
	{
		out << YAML::Key << "Name" << YAML::Value << name->Name;
	}
	out << YAML::EndMap; // NameComponent
}

void SceneSerializer::SerializeShaderComponent(YAML::Emitter& out, const ShaderComponent* shader) {
	out << YAML::Key << "ShaderComponent" << YAML::Value << YAML::BeginMap; // ShaderComponent
	{
		out << YAML::Key << "Shader" << YAML::Value << ""; // TODO: Serialize Shader Reference
	}
	out << YAML::EndMap; // ShaderComponent
}

void SceneSerializer::SerializeTransformComponent(YAML::Emitter& out, const TransformComponent* transform) {
	out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent
	{
		out << YAML::Key << "Position" << YAML::Value << transform->Position;
		out << YAML::Key << "Rotation" << YAML::Value << transform->Rotation;
		out << YAML::Key << "Scale" << YAML::Value << transform->Scale;
	}
	out << YAML::EndMap; // TransformComponent
}

void SceneSerializer::DeserializeEntity(const YAML::Node& entityNode) {
	if (const auto& idNode = entityNode["ID"]) {
		UUID entityID = static_cast<UUID>(idNode.as<uint64_t>());
		Entity entity = m_Scene.CreateEntity(entityID);

		if (const auto& componentsNode = entityNode["Components"]) {
			if (const auto& nameComponentNode = componentsNode["NameComponent"]) {
				DeserializeNameComponent(nameComponentNode, entity);
			}

			if (const auto& transformComponentNode = componentsNode["TransformComponent"]) {
				DeserializeTransformComponent(transformComponentNode, entity);
			}

			if (const auto& cameraComponentNode = componentsNode["CameraComponent"]) {
				DeserializeCameraComponent(cameraComponentNode, entity);
			}

			if (const auto& lightComponentNode = componentsNode["LightComponent"]) {
				DeserializeLightComponent(lightComponentNode, entity);
			}

			if (const auto& materialComponentNode = componentsNode["MaterialComponent"]) {
				DeserializeMaterialComponent(materialComponentNode, entity);
			}

			if (const auto& meshComponentNode = componentsNode["MeshComponent"]) {
				DeserializeMeshComponent(meshComponentNode, entity);
			}

			if (const auto& shaderComponentNode = componentsNode["ShaderComponent"]) {
				DeserializeShaderComponent(shaderComponentNode, entity);
			}
		}
	}
}

void SceneSerializer::DeserializeCameraComponent(const YAML::Node& cameraNode, Entity& entity) {
	if (entity.HasComponent<CameraComponent>()) {
		Log::Warning("SceneSerializer::DeserializeCameraComponent - Entity already has a CameraComponent, overwriting");
		entity.RemoveComponent<CameraComponent>();
	}

	auto& camera = entity.AddComponent<CameraComponent>();

	if (const auto& fovNode = cameraNode["FOV"]) {
		camera.FOV = fovNode.as<float>();
	}

	if (const auto& nearClipNode = cameraNode["NearClip"]) {
		camera.NearClip = nearClipNode.as<float>();
	}

	if (const auto& farClipNode = cameraNode["FarClip"]) {
		camera.FarClip = farClipNode.as<float>();
	}

	if (const auto& primaryNode = cameraNode["Primary"]) {
		camera.Primary = primaryNode.as<bool>();
	}
}

void SceneSerializer::DeserializeLightComponent(const YAML::Node& lightNode, Entity& entity) {
	if (entity.HasComponent<LightComponent>()) {
		Log::Warning("SceneSerializer::DeserializeLightComponent - Entity already has a LightComponent, overwriting");
		entity.RemoveComponent<LightComponent>();
	}

	auto& light = entity.AddComponent<LightComponent>();

	if (const auto& typeNode = lightNode["Type"]) {
		light.Type = Utilities::StringToLightType(typeNode.as<std::string>());
	}

	if (const auto& colorNode = lightNode["Color"]) {
		light.Color = colorNode.as<glm::vec3>();
	}

	if (const auto& intensityNode = lightNode["Intensity"]) {
		light.Intensity = intensityNode.as<float>();
	}

	if (const auto& rangeNode = lightNode["Range"]) {
		light.Range = rangeNode.as<float>();
	}

	if (const auto& directionNode = lightNode["Direction"]) {
		light.Direction = directionNode.as<glm::vec3>();
	}

	if (const auto& constantNode = lightNode["Constant"]) {
		light.Constant = constantNode.as<float>();
	}

	if (const auto& linearNode = lightNode["Linear"]) {
		light.Linear = linearNode.as<float>();
	}

	if (const auto& quadraticNode = lightNode["Quadratic"]) {
		light.Quadratic = quadraticNode.as<float>();
	}

	if (const auto& innerConeNode = lightNode["InnerCone"]) {
		light.InnerCone = innerConeNode.as<float>();
	}

	if (const auto& outerConeNode = lightNode["OuterCone"]) {
		light.OuterCone = outerConeNode.as<float>();
	}
}

void SceneSerializer::DeserializeMaterialComponent(const YAML::Node& materialNode, Entity& entity) {
	if (entity.HasComponent<MaterialComponent>()) {
		Log::Warning("SceneSerializer::DeserializeMaterialComponent - Entity already has a MaterialComponent, overwriting");
		entity.RemoveComponent<MaterialComponent>();
	}

	auto& material = entity.AddComponent<MaterialComponent>();

	if (const auto& materialFilepathNode = materialNode["MaterialFilepath"]) {
		material.MaterialFilepath = materialFilepathNode.as<std::string>();

		if (!material.MaterialFilepath.empty()) {
			MaterialSerializer materialSerializer(material.Material);
			
			if (!materialSerializer.Deserialize(material.MaterialFilepath)) {
				Log::Warning("SceneSerializer::DeserializeMaterialComponent - Couldn't deserialize Material: " + material.MaterialFilepath.string());
			}
		}
	}
}

void SceneSerializer::DeserializeMeshComponent(const YAML::Node& meshNode, Entity& entity) {
	if (entity.HasComponent<MeshComponent>()) {
		Log::Warning("SceneSerializer::DeserializeMeshComponent - Entity already has a MeshComponent, overwriting");
		entity.RemoveComponent<MeshComponent>();
	}

	auto& mesh = entity.AddComponent<MeshComponent>();

	if (const auto& meshFilepathNode = meshNode["MeshFilepath"]) {
		mesh.MeshFilepath = meshFilepathNode.as<std::string>();
	}

	if (const auto& castShadowsNode = meshNode["CastShadows"]) {
		mesh.CastShadows = castShadowsNode.as<bool>();
	}

	if (const auto& receiveShadowsNode = meshNode["ReceiveShadows"]) {
		mesh.ReceiveShadows = receiveShadowsNode.as<bool>();
	}
}

void SceneSerializer::DeserializeNameComponent(const YAML::Node& nameNode, Entity& entity) {
	if (entity.HasComponent<NameComponent>()) {
		Log::Warning("SceneSerializer::DeserializeNameComponent - Entity already has a NameComponent, overwriting");
		entity.RemoveComponent<NameComponent>();
	}

	auto& name = entity.AddComponent<NameComponent>();

	if (const auto& nameValueNode = nameNode["Name"]) {
		name.Name = nameValueNode.as<std::string>();
	}
}

void SceneSerializer::DeserializeShaderComponent(const YAML::Node& shaderNode, Entity& entity) {
	if (entity.HasComponent<ShaderComponent>()) {
		Log::Warning("SceneSerializer::DeserializeShaderComponent - Entity already has a ShaderComponent, overwriting");
		entity.RemoveComponent<ShaderComponent>();
	}

	auto& shader = entity.AddComponent<ShaderComponent>();

	if (const auto& shaderRefNode = shaderNode["Shader"]) {
		// TODO: Deserialize Shader Reference
	}
}

void SceneSerializer::DeserializeTransformComponent(const YAML::Node& transformNode, Entity& entity) {
	if (entity.HasComponent<TransformComponent>()) {
		Log::Warning("SceneSerializer::DeserializeTransformComponent - Entity already has a TransformComponent, overwriting");
		entity.RemoveComponent<TransformComponent>();
	}

	auto& transform = entity.AddComponent<TransformComponent>();

	if (const auto& positionNode = transformNode["Position"]) {
		transform.Position = positionNode.as<glm::vec3>();
	}

	if (const auto& rotationNode = transformNode["Rotation"]) {
		transform.Rotation = rotationNode.as<glm::vec3>();
	}

	if (const auto& scaleNode = transformNode["Scale"]) {
		transform.Scale = scaleNode.as<glm::vec3>();
	}
}

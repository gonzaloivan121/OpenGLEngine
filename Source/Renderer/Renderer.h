#pragma once

#include "Renderer/RenderCommand.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Shader.h"

#include "Core/Mesh/Mesh.h"
#include "ECS/Scene.h"

#include <filesystem>
#include <unordered_map>
#include <string>

class Renderer {
public:
	struct SceneData {
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Projection = glm::mat4(1.0f);
		glm::mat4 ViewProjection = glm::mat4(1.0f);
		glm::vec3 CameraPosition = glm::vec3(0.0f);
	};

	static void Init();
	static void Shutdown();

	static void Begin();
	static void End();

	static void Submit(Scene& scene);
	static void ExportFrame(const std::filesystem::path& filepath);

	static Ref<Framebuffer> GetFramebuffer() { return s_Framebuffer; }
private:
	static void InitFramebuffer();
	static void InitGBuffer();
	static void InitFullscreenQuad();
	static void InitShaders();

	static void GeometryPass(Scene& scene);
	static void BackgroundPass();
	static void LightingPass(Scene& scene);

	static Ref<Shader> GetOrLoadShader(const std::filesystem::path& path);
private:
	inline static Ref<Framebuffer> s_Framebuffer = nullptr;
	inline static Ref<Framebuffer> s_GBuffer = nullptr;

	inline static Ref<Mesh> m_QuadMesh = nullptr;
	inline static Ref<Shader> m_Shader = nullptr;
	inline static Ref<Shader> m_DefaultGeometryShader = nullptr;
	inline static Ref<Shader> m_LightingShader = nullptr;

	inline static SceneData s_SceneData;
	inline static std::unordered_map<std::string, Ref<Shader>> m_ShaderCache;
};
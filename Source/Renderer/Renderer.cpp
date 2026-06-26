#include "Renderer.h"

#include "Core/Application/Application.h"
#include "Core/Log/Log.h"
#include "Core/Settings/SettingsManager.h"
#include "ECS/Components.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <array>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "stb_image_write.h"

static constexpr int MAX_LIGHTS = 16;

namespace {
	struct ObjIndex {
		int Position = -1;
		int UV = -1;
		int Normal = -1;

		bool operator==(const ObjIndex& other) const {
			return Position == other.Position && UV == other.UV && Normal == other.Normal;
		}
	};

	struct ObjIndexHash {
		size_t operator()(const ObjIndex& index) const {
			size_t h1 = std::hash<int>{}(index.Position);
			size_t h2 = std::hash<int>{}(index.UV);
			size_t h3 = std::hash<int>{}(index.Normal);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

	std::filesystem::path ResolveMeshPath(const std::filesystem::path& filepath) {
		if (filepath.empty()) {
			return {};
		}

		if (std::filesystem::exists(filepath)) {
			return filepath;
		}

		std::filesystem::path probe = filepath;
		for (int i = 0; i < 8; ++i) {
			probe = std::filesystem::path("..") / probe;
			if (std::filesystem::exists(probe)) {
				return probe;
			}
		}

		return {};
	}

	int ParseOBJIndex(const std::string& token, size_t listSize) {
		if (token.empty()) {
			return -1;
		}

		const int idx = std::stoi(token);
		if (idx > 0) {
			return idx - 1;
		}

		if (idx < 0) {
			const int resolved = static_cast<int>(listSize) + idx;
			return resolved >= 0 ? resolved : -1;
		}

		return -1;
	}

	bool ParseOBJFaceVertex(const std::string& token, ObjIndex& out, size_t posCount, size_t uvCount, size_t normalCount) {
		std::array<std::string, 3> parts = {};
		size_t partIndex = 0;
		size_t start = 0;

		while (start <= token.size() && partIndex < 3) {
			size_t slash = token.find('/', start);
			if (slash == std::string::npos) {
				parts[partIndex++] = token.substr(start);
				break;
			}

			parts[partIndex++] = token.substr(start, slash - start);
			start = slash + 1;
		}

		out.Position = ParseOBJIndex(parts[0], posCount);
		out.UV = ParseOBJIndex(parts[1], uvCount);
		out.Normal = ParseOBJIndex(parts[2], normalCount);

		return out.Position >= 0;
	}

	Ref<Mesh> TryLoadOBJMesh(const std::filesystem::path& filepath) {
		auto buildFallbackCube = []() -> Ref<Mesh> {
			std::vector<MeshVertex> vertices = {
				{ glm::vec3(-0.5f, -0.5f, -0.5f) },
				{ glm::vec3( 0.5f, -0.5f, -0.5f) },
				{ glm::vec3( 0.5f,  0.5f, -0.5f) },
				{ glm::vec3(-0.5f,  0.5f, -0.5f) },
				{ glm::vec3(-0.5f, -0.5f,  0.5f) },
				{ glm::vec3( 0.5f, -0.5f,  0.5f) },
				{ glm::vec3( 0.5f,  0.5f,  0.5f) },
				{ glm::vec3(-0.5f,  0.5f,  0.5f) }
			};

			std::vector<uint32_t> indices = {
				// Front
				4, 5, 6, 6, 7, 4,
				// Back
				1, 0, 3, 3, 2, 1,
				// Left
				0, 4, 7, 7, 3, 0,
				// Right
				5, 1, 2, 2, 6, 5,
				// Top
				3, 7, 6, 6, 2, 3,
				// Bottom
				0, 1, 5, 5, 4, 0
			};

			Ref<Mesh> fallback = Mesh::Create(vertices, indices);
			fallback->RecalculateNormals();
			return fallback;
		};

		const std::filesystem::path resolvedPath = ResolveMeshPath(filepath);
		if (resolvedPath.empty()) {
			Log::Warning("Renderer::TryLoadOBJMesh - Mesh file not found: " + filepath.string());
			Log::Warning("Renderer::TryLoadOBJMesh - Using fallback cube mesh");
			return buildFallbackCube();
		}

		std::ifstream file(resolvedPath);
		if (!file.is_open()) {
			Log::Warning("Renderer::TryLoadOBJMesh - Failed to open mesh file: " + resolvedPath.string());
			Log::Warning("Renderer::TryLoadOBJMesh - Using fallback cube mesh");
			return buildFallbackCube();
		}

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;

		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<ObjIndex, uint32_t, ObjIndexHash> vertexLookup;

		std::string line;
		while (std::getline(file, line)) {
			if (line.size() < 2) {
				continue;
			}

			if (line.rfind("v ", 0) == 0) {
				std::istringstream stream(line.substr(2));
				glm::vec3 p;
				stream >> p.x >> p.y >> p.z;
				positions.push_back(p);
			} else if (line.rfind("vt ", 0) == 0) {
				std::istringstream stream(line.substr(3));
				glm::vec2 uv;
				stream >> uv.x >> uv.y;
				uvs.push_back(uv);
			} else if (line.rfind("vn ", 0) == 0) {
				std::istringstream stream(line.substr(3));
				glm::vec3 n;
				stream >> n.x >> n.y >> n.z;
				normals.push_back(n);
			} else if (line.rfind("f ", 0) == 0) {
				std::istringstream stream(line.substr(2));
				std::vector<uint32_t> faceIndices;
				std::string token;

				while (stream >> token) {
					ObjIndex objIndex;
					if (!ParseOBJFaceVertex(token, objIndex, positions.size(), uvs.size(), normals.size())) {
						continue;
					}

					auto found = vertexLookup.find(objIndex);
					if (found != vertexLookup.end()) {
						faceIndices.push_back(found->second);
						continue;
					}

					MeshVertex v;
					v.Position = positions[objIndex.Position];
					if (objIndex.UV >= 0 && objIndex.UV < static_cast<int>(uvs.size())) {
						v.UV = uvs[objIndex.UV];
						v.UV2 = v.UV;
					}
					if (objIndex.Normal >= 0 && objIndex.Normal < static_cast<int>(normals.size())) {
						v.Normal = normals[objIndex.Normal];
					}

					const uint32_t newIndex = static_cast<uint32_t>(vertices.size());
					vertices.push_back(v);
					vertexLookup[objIndex] = newIndex;
					faceIndices.push_back(newIndex);
				}

				for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
					indices.push_back(faceIndices[0]);
					indices.push_back(faceIndices[i]);
					indices.push_back(faceIndices[i + 1]);
				}
			}
		}

		if (vertices.empty() || indices.empty()) {
			Log::Warning("Renderer::TryLoadOBJMesh - Parsed mesh has no geometry: " + resolvedPath.string());
			Log::Warning("Renderer::TryLoadOBJMesh - Using fallback cube mesh");
			return buildFallbackCube();
		}

		Ref<Mesh> mesh = Mesh::Create(vertices, indices);
		if (normals.empty()) {
			mesh->RecalculateNormals();
		}

		return mesh;
	}
}

void Renderer::Init() {
	Log::Trace("Renderer::Init - Initializing the Renderer");
	RenderCommand::Init();

	InitFramebuffer();
	InitGBuffer();
	InitFullscreenQuad();
	InitShaders();

	RenderCommand::EnableDepthTest(true);
}

void Renderer::Shutdown() {
	Log::Trace("Renderer::Shutdown - Shutting down the Renderer");

	m_ShaderCache.clear();
	m_LightingShader.reset();
	m_DefaultGeometryShader.reset();
	m_Shader.reset();
	m_QuadMesh.reset();
	s_GBuffer.reset();
	s_Framebuffer.reset();
}

void Renderer::Begin() {
	RenderCommand::ProcessDeletionQueue();
}

void Renderer::End() {
	s_Framebuffer->Unbind();
}

void Renderer::Submit(Scene& scene) {
	// Sync G-Buffer dimensions to the output framebuffer
	const uint32_t fbWidth  = s_Framebuffer->GetWidth();
	const uint32_t fbHeight = s_Framebuffer->GetHeight();
	if (s_GBuffer->GetWidth() != fbWidth || s_GBuffer->GetHeight() != fbHeight) {
		s_GBuffer->Resize(fbWidth, fbHeight);
	}

	GeometryPass(scene);
	BackgroundPass();
	LightingPass(scene);
}

void Renderer::GeometryPass(Scene& scene) {
	s_GBuffer->Bind();
	RenderCommand::Clear();
	RenderCommand::EnableDepthTest(true);
	RenderCommand::EnableCullFace(true);

	const float width  = static_cast<float>(s_GBuffer->GetWidth());
	const float height = static_cast<float>(s_GBuffer->GetHeight());
	const glm::vec2 resolution(width, height);

	// Find the primary camera entity
	glm::mat4 view       = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), width / height, 0.1f, 1000.0f);
	glm::vec3 camPos     = glm::vec3(0.0f);

	for (const auto& id : scene.GetEntityIDs()) {
		Entity entity = scene.GetEntity(id);
		const auto* cc = entity.GetComponent<CameraComponent>();
		if (!cc || !cc->Primary) continue;
		const auto* tc = entity.GetComponent<TransformComponent>();
		if (!tc) continue;
		camPos     = tc->Position;
		view       = glm::inverse(tc->GetTransform());
		projection = cc->GetProjection(width / height);
		break;
	}

	s_SceneData.View           = view;
	s_SceneData.Projection     = projection;
	s_SceneData.ViewProjection = projection * view;
	s_SceneData.CameraPosition = camPos;

	// Draw each entity that has a mesh and a shader
	for (const auto& id : scene.GetEntityIDs()) {
		Entity entity = scene.GetEntity(id);
		auto* mesh = entity.GetComponent<MeshComponent>();

		const auto* material = entity.GetComponent<MaterialComponent>();
		const auto* transform = entity.GetComponent<TransformComponent>();

		if (!mesh || !transform) {
			continue;
		}

		if (!mesh->Mesh && !mesh->MeshFilepath.empty()) {
			mesh->Mesh = TryLoadOBJMesh(mesh->MeshFilepath);
		}

		if (!mesh->Mesh) {
			continue;
		}

		Ref<Shader> shader = nullptr;
		if (material && !material->Material.ShaderFilepath.empty()) {
			shader = GetOrLoadShader(material->Material.ShaderFilepath);
		} else {
			shader = m_DefaultGeometryShader;
		}

		if (!shader) {
			continue;
		}

		shader->Bind();
		shader->SetUniform("u_Model",          transform->GetTransform());
		shader->SetUniform("u_View",           s_SceneData.View);
		shader->SetUniform("u_Projection",     s_SceneData.Projection);
		shader->SetUniform("u_ViewProjection", s_SceneData.ViewProjection);
		shader->SetUniform("u_CameraPosition", s_SceneData.CameraPosition);
		shader->SetUniform("u_Resolution",     resolution);

		if (material) {
			shader->SetUniform("u_Albedo", material->Material.Albedo);
			shader->SetUniform("u_Metallic", material->Material.Metallic);
			shader->SetUniform("u_Roughness", material->Material.Roughness);
			shader->SetUniform("u_AmbientOcclusion", material->Material.AmbientOcclusion);
			shader->SetUniform("u_EmissionColor", material->Material.Emission.Color);
			shader->SetUniform("u_EmissionIntensity", material->Material.Emission.Intensity);
			shader->SetUniform("u_EmissionEnabled", material->Material.Emission.Enabled);
		}

		if (const auto& vertexArray = mesh->Mesh->GetVertexArray()) {
			RenderCommand::DrawIndexed(vertexArray);
		}
	}

	s_GBuffer->Unbind();
}

void Renderer::BackgroundPass() {
	s_Framebuffer->Bind();
	RenderCommand::Clear();

	if (!m_Shader || !m_QuadMesh) {
		return;
	}

	RenderCommand::EnableDepthTest(false);

	m_Shader->Bind();
	m_Shader->SetUniform("u_Resolution", glm::vec2(
		static_cast<float>(s_Framebuffer->GetWidth()),
		static_cast<float>(s_Framebuffer->GetHeight())
	));

	if (const auto& vertexArray = m_QuadMesh->GetVertexArray()) {
		RenderCommand::DrawIndexed(vertexArray);
	}

	RenderCommand::EnableDepthTest(true);
}

void Renderer::LightingPass(Scene& scene) {
	if (!m_LightingShader || !m_QuadMesh) {
		return;
	}

	// Bind G-Buffer textures to sampler units 0-3
	if (const auto& pos = s_GBuffer->GetColorAttachment(0)) pos->Bind(0);
	if (const auto& nor = s_GBuffer->GetColorAttachment(1)) nor->Bind(1);
	if (const auto& alb = s_GBuffer->GetColorAttachment(2)) alb->Bind(2);
	if (const auto& mat = s_GBuffer->GetColorAttachment(3)) mat->Bind(3);

	m_LightingShader->Bind();
	m_LightingShader->SetUniform("u_gPosition", 0);
	m_LightingShader->SetUniform("u_gNormal",   1);
	m_LightingShader->SetUniform("u_gAlbedo",   2);
	m_LightingShader->SetUniform("u_gMaterial", 3);
	m_LightingShader->SetUniform("u_CameraPosition", s_SceneData.CameraPosition);
	m_LightingShader->SetUniform("u_Resolution", glm::vec2(
		static_cast<float>(s_Framebuffer->GetWidth()),
		static_cast<float>(s_Framebuffer->GetHeight())
	));

	// Collect lights from scene entities
	std::vector<glm::vec3> dirDirections, dirColors;
	std::vector<float>     dirIntensities;

	std::vector<glm::vec3> pointPositions, pointColors;
	std::vector<float>     pointIntensities, pointRanges, pointLinears, pointQuadratics;

	std::vector<glm::vec3> spotPositions, spotDirections, spotColors;
	std::vector<float>     spotIntensities, spotInnerCones, spotOuterCones;

	for (const auto& id : scene.GetEntityIDs()) {
		Entity entity = scene.GetEntity(id);
		const auto* light = entity.GetComponent<LightComponent>();
		const auto* transform = entity.GetComponent<TransformComponent>();

		if (!light || !transform) {
			continue;
		}

		switch (light->Type) {
			case LightType::Directional:
				if (static_cast<int>(dirDirections.size()) < MAX_LIGHTS) {
					dirDirections.push_back(light->Direction);
					dirColors.push_back(light->Color);
					dirIntensities.push_back(light->Intensity);
				}
				break;
			case LightType::Point:
				if (static_cast<int>(pointPositions.size()) < MAX_LIGHTS) {
					pointPositions.push_back(transform->Position);
					pointColors.push_back(light->Color);
					pointIntensities.push_back(light->Intensity);
					pointRanges.push_back(light->Range);
					pointLinears.push_back(light->Linear);
					pointQuadratics.push_back(light->Quadratic);
				}
				break;
			case LightType::Spot:
				if (static_cast<int>(spotPositions.size()) < MAX_LIGHTS) {
					spotPositions.push_back(transform->Position);
					spotDirections.push_back(light->Direction);
					spotColors.push_back(light->Color);
					spotIntensities.push_back(light->Intensity);
					spotInnerCones.push_back(light->InnerCone);
					spotOuterCones.push_back(light->OuterCone);
				}
				break;
		}
	}

	// Upload directional lights
	m_LightingShader->SetUniform("u_DirLightCount", static_cast<int>(dirDirections.size()));
	if (!dirDirections.empty()) {
		m_LightingShader->UploadUniformVec3Array("u_DirLightDirection", dirDirections.data(), static_cast<uint32_t>(dirDirections.size()));
		m_LightingShader->UploadUniformVec3Array("u_DirLightColor",     dirColors.data(),     static_cast<uint32_t>(dirColors.size()));
		m_LightingShader->UploadUniformFloatArray("u_DirLightIntensity", dirIntensities.data(), static_cast<uint32_t>(dirIntensities.size()));
	}

	// Upload point lights
	m_LightingShader->SetUniform("u_PointLightCount", static_cast<int>(pointPositions.size()));
	if (!pointPositions.empty()) {
		m_LightingShader->UploadUniformVec3Array("u_PointLightPosition",  pointPositions.data(),  static_cast<uint32_t>(pointPositions.size()));
		m_LightingShader->UploadUniformVec3Array("u_PointLightColor",     pointColors.data(),     static_cast<uint32_t>(pointColors.size()));
		m_LightingShader->UploadUniformFloatArray("u_PointLightIntensity",  pointIntensities.data(),  static_cast<uint32_t>(pointIntensities.size()));
		m_LightingShader->UploadUniformFloatArray("u_PointLightRange",      pointRanges.data(),      static_cast<uint32_t>(pointRanges.size()));
		m_LightingShader->UploadUniformFloatArray("u_PointLightLinear",     pointLinears.data(),     static_cast<uint32_t>(pointLinears.size()));
		m_LightingShader->UploadUniformFloatArray("u_PointLightQuadratic",  pointQuadratics.data(),  static_cast<uint32_t>(pointQuadratics.size()));
	}

	// Upload spot lights
	m_LightingShader->SetUniform("u_SpotLightCount", static_cast<int>(spotPositions.size()));
	if (!spotPositions.empty()) {
		m_LightingShader->UploadUniformVec3Array("u_SpotLightPosition",  spotPositions.data(),  static_cast<uint32_t>(spotPositions.size()));
		m_LightingShader->UploadUniformVec3Array("u_SpotLightDirection", spotDirections.data(), static_cast<uint32_t>(spotDirections.size()));
		m_LightingShader->UploadUniformVec3Array("u_SpotLightColor",     spotColors.data(),     static_cast<uint32_t>(spotColors.size()));
		m_LightingShader->UploadUniformFloatArray("u_SpotLightIntensity",  spotIntensities.data(),  static_cast<uint32_t>(spotIntensities.size()));
		m_LightingShader->UploadUniformFloatArray("u_SpotLightInnerCone",  spotInnerCones.data(),   static_cast<uint32_t>(spotInnerCones.size()));
		m_LightingShader->UploadUniformFloatArray("u_SpotLightOuterCone",  spotOuterCones.data(),   static_cast<uint32_t>(spotOuterCones.size()));
	}

	RenderCommand::EnableDepthTest(false);

	if (const auto& vertexArray = m_QuadMesh->GetVertexArray()) {
		RenderCommand::DrawIndexed(vertexArray);
	}

	RenderCommand::EnableDepthTest(true);
}

Ref<Shader> Renderer::GetOrLoadShader(const std::filesystem::path& path) {
	const std::string key = path.string();
	const auto it = m_ShaderCache.find(key);

	if (it != m_ShaderCache.end()) {
		return it->second;
	}

	Ref<Shader> shader = Shader::Create(path);
	m_ShaderCache[key] = shader;
	
	return shader;
}

void Renderer::ExportFrame(const std::filesystem::path& filepath) {
	if (!s_Framebuffer) {
		Log::Error("Renderer::ExportFrame - Cannot export, framebuffer is null.");
		return;
	}

	uint32_t width = s_Framebuffer->GetWidth();
	uint32_t height = s_Framebuffer->GetHeight();

	if (width == 0 || height == 0) {
		Log::Warning("Renderer::ExportFrame - Cannot export a frame with zero size.");
		return;
	}

	// Create a buffer in the CPU memory to store the pixels.
	// The size is width * height * 4 (because we use RGBA, 4 channels of 1 byte).
	std::vector<uint8_t> pixels(width * height * 4);

	// Tell OpenGL that we want to read from our framebuffer.
	s_Framebuffer->Bind();
	// Read the pixels from the framebuffer to our CPU buffer.
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	// Stop using the framebuffer.
	s_Framebuffer->Unbind();

	// Tell STB to flip the image vertically when saving it.
	// This is necessary because the OpenGL coordinate system(0, 0 down) is
	// inverted compared to most image formats(0, 0 up).
	stbi_flip_vertically_on_write(1);

	const auto& exportSettings = SettingsManager::Get().Export;
	const std::string pathStr = filepath.string();

	switch (exportSettings.ImageFormat) {
		case ExportImageFormat::JPEG: {
			// JPEG does not support an alpha channel; drop it to RGB first.
			std::vector<uint8_t> rgb(width * height * 3);
			for (uint32_t i = 0; i < width * height; ++i) {
				rgb[i * 3 + 0] = pixels[i * 4 + 0];
				rgb[i * 3 + 1] = pixels[i * 4 + 1];
				rgb[i * 3 + 2] = pixels[i * 4 + 2];
			}
			stbi_write_jpg(pathStr.c_str(), width, height, 3, rgb.data(), exportSettings.ImageQuality);
			break;
		}
		case ExportImageFormat::BMP:
			stbi_write_bmp(pathStr.c_str(), width, height, 4, pixels.data());
			break;
		case ExportImageFormat::PNG:
		default:
			stbi_write_png(pathStr.c_str(), width, height, 4, pixels.data(), width * 4);
			break;
	}
}

void Renderer::InitFramebuffer() {
	Log::Trace("Renderer::InitFramebuffer - Initializing the Framebuffer");
	Log::Trace("Renderer::InitFramebuffer - Setting up the Framebuffer Texture Specification");

	TextureSpecification textureSpec;
	textureSpec.Width = Application::Get().GetWidth();
	textureSpec.Height = Application::Get().GetHeight();
	textureSpec.Format = TextureFormat::RGBA16F;
	textureSpec.MinFilter = TextureFilter::Linear;
	textureSpec.MagFilter = TextureFilter::Linear;
	textureSpec.WrapS = TextureWrap::ClampToEdge;
	textureSpec.WrapT = TextureWrap::ClampToEdge;
	textureSpec.GenerateMips = false;

	Log::Trace("Renderer::InitFramebuffer - Setting up the Framebuffer");
	FramebufferSpecification fbSpec;
	fbSpec.Width = textureSpec.Width;
	fbSpec.Height = textureSpec.Height;
	fbSpec.ColorAttachments.push_back(textureSpec);
	fbSpec.HasDepthAttachment = true;
	fbSpec.DepthAttachmentSpecification.Format = TextureFormat::Depth24Stencil8;

	Log::Trace("Renderer::InitFramebuffer - Creating the Framebuffer");
	s_Framebuffer = Framebuffer::Create(fbSpec);
}

void Renderer::InitGBuffer() {
	Log::Trace("Renderer::InitGBuffer - Initializing the G-Buffer");

	const uint32_t width  = Application::Get().GetWidth();
	const uint32_t height = Application::Get().GetHeight();

	FramebufferSpecification gbSpec;
	gbSpec.Width  = width;
	gbSpec.Height = height;

	// Attachment 0: world-space position (RGBA16F, W=1 signals geometry present)
	TextureSpecification posSpec;
	posSpec.Format     = TextureFormat::RGBA16F;
	posSpec.MinFilter  = TextureFilter::Nearest;
	posSpec.MagFilter  = TextureFilter::Nearest;
	posSpec.WrapS      = TextureWrap::ClampToEdge;
	posSpec.WrapT      = TextureWrap::ClampToEdge;
	posSpec.GenerateMips = false;
	gbSpec.ColorAttachments.push_back(posSpec);

	// Attachment 1: world-space normals (RGBA16F)
	TextureSpecification normalSpec = posSpec;
	gbSpec.ColorAttachments.push_back(normalSpec);

	// Attachment 2: albedo (RGB) + roughness (A) as RGBA8
	TextureSpecification albedoSpec;
	albedoSpec.Format     = TextureFormat::RGBA8;
	albedoSpec.MinFilter  = TextureFilter::Nearest;
	albedoSpec.MagFilter  = TextureFilter::Nearest;
	albedoSpec.WrapS      = TextureWrap::ClampToEdge;
	albedoSpec.WrapT      = TextureWrap::ClampToEdge;
	albedoSpec.GenerateMips = false;
	gbSpec.ColorAttachments.push_back(albedoSpec);

	// Attachment 3: metallic (R) + AO (G) + emission (B) as RGBA8
	TextureSpecification materialSpec = albedoSpec;
	gbSpec.ColorAttachments.push_back(materialSpec);

	gbSpec.HasDepthAttachment = true;
	gbSpec.DepthAttachmentSpecification.Format = TextureFormat::Depth24Stencil8;

	s_GBuffer = Framebuffer::Create(gbSpec);
}

void Renderer::InitFullscreenQuad() {
	Log::Trace("Renderer::InitFullscreenQuad - Initializing Fullscreen Quad Mesh");

	m_QuadMesh = Mesh::Create();
	m_QuadMesh->SetVertices({
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3( 1.0f, -1.0f, 0.0f),
		glm::vec3( 1.0f,  1.0f, 0.0f),
		glm::vec3(-1.0f,  1.0f, 0.0f)
	});
	m_QuadMesh->SetUVs({
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f)
	});
	m_QuadMesh->SetUV2s({
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f)
	});
	m_QuadMesh->SetTangents({
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
	});
	m_QuadMesh->SetTriangles({ 0, 1, 2, 2, 3, 0 });
	m_QuadMesh->RecalculateNormals();
}

void Renderer::InitShaders() {
	Log::Trace("Renderer::InitShaders - Initializing Shaders");

	m_Shader = Shader::CreateGraphics(
		"Internal/Shaders/Default/Default.vert",
		"Internal/Shaders/Default/Default.frag"
	);

	m_DefaultGeometryShader = Shader::CreateGraphics(
		"Internal/Shaders/GBuffer/GBuffer.vert",
		"Internal/Shaders/GBuffer/GBuffer.frag"
	);

	m_LightingShader = Shader::CreateGraphics(
		"Internal/Shaders/Lighting/Lighting.vert",
		"Internal/Shaders/Lighting/Lighting.frag"
	);
}

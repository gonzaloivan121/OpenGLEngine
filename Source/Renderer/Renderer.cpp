#include "Renderer.h"

#include "Core/Application/Application.h"
#include "Core/Log/Log.h"
#include "Core/Settings/SettingsManager.h"
#include <glm/gtc/type_ptr.hpp>

#include "stb_image_write.h"

void Renderer::Init() {
	Log::Trace("Renderer::Init - Initializing the Renderer");
	RenderCommand::Init();

	InitFramebuffer();
	InitVertexArray();
	InitShader();

	RenderCommand::EnableDepthTest(true);
}

void Renderer::Shutdown() {
	Log::Trace("Renderer::Shutdown - Shutting down the Renderer");

	m_TriangleShader.reset();
	m_Shader.reset();
	m_TriangleMesh.reset();
	m_QuadMesh.reset();
	s_Framebuffer.reset();
}

void Renderer::Begin() {
	RenderCommand::ProcessDeletionQueue();

	s_Framebuffer->Bind();

	RenderCommand::Clear();
}

void Renderer::End() {
	// Unbind our framebuffer
	s_Framebuffer->Unbind();
}

void Renderer::Submit(const Scene& scene) {
	if (!m_Shader || !m_QuadMesh) {
		return;
	}

	const float& width = (float)s_Framebuffer->GetWidth();
	const float& height = (float)s_Framebuffer->GetHeight();
	const glm::vec2 resolution = glm::vec2(width, height);

	m_Shader->Bind();
	m_Shader->SetUniform("u_Resolution", resolution);
	const Ref<VertexArray>& quadVA = m_QuadMesh->GetVertexArray();
	if (quadVA) {
		RenderCommand::DrawIndexed(quadVA);
	}

	if (m_TriangleShader && m_TriangleMesh) {
		m_TriangleShader->Bind();
		const Ref<VertexArray>& triangleVA = m_TriangleMesh->GetVertexArray();
		if (triangleVA) {
			RenderCommand::DrawIndexed(triangleVA);
		}
	}
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
	fbSpec.ColorAttachmentSpecification = textureSpec;
	fbSpec.HasDepthAttachment = true;
	fbSpec.DepthAttachmentSpecification.Format = TextureFormat::Depth24Stencil8;

	Log::Trace("Renderer::InitFramebuffer - Creating the Framebuffer");
	s_Framebuffer = Framebuffer::Create(fbSpec);
}

void Renderer::InitVertexArray() {
	Log::Trace("Renderer::InitFramebuffer - Initializing Fullscreen Mesh");

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

	Log::Trace("Renderer::InitFramebuffer - Initializing Triangle Mesh");

	std::vector<MeshVertex> triangleVertices = {
		{ glm::vec3( 0.0f,  0.65f, -0.25f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f), glm::vec2(0.5f, 1.0f), glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ glm::vec3( 0.65f, -0.55f, -0.25f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.2f, 1.0f, 0.2f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ glm::vec3(-0.65f, -0.55f, -0.25f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(0.2f, 0.4f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) }
	};

	m_TriangleMesh = Mesh::Create(triangleVertices, { 0, 1, 2 });
}

void Renderer::InitShader() {
	Log::Trace("Renderer::InitFramebuffer - Initializing Shader");

	m_Shader = Shader::CreateGraphics(
		"Internal/Shaders/Default/Default.vert",
		"Internal/Shaders/Default/Default.frag"
	);

	m_TriangleShader = Shader::CreateGraphics(
		"Internal/Shaders/Triangle/Triangle.vert",
		"Internal/Shaders/Triangle/Triangle.frag"
	);
}

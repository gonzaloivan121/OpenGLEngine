#pragma once

#include "Core/Core.h"

#include "Renderer/Texture.h"

#include <vector>

struct FramebufferSpecification {
	uint32_t Width = 0;
	uint32_t Height = 0;
	std::vector<TextureSpecification> ColorAttachments;
	TextureSpecification DepthAttachmentSpecification;
	bool HasDepthAttachment = false;
};

class Framebuffer {
public:
	virtual ~Framebuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void Resize(uint32_t width, uint32_t height) = 0;

	virtual Ref<Texture2D> GetColorAttachment(uint32_t index = 0) const = 0;
	virtual Ref<Texture2D> GetDepthAttachment() const = 0;

	virtual uint32_t GetHandle() const = 0;
	virtual uint32_t GetColorAttachmentCount() const = 0;

	virtual const FramebufferSpecification& GetSpecification() const = 0;

	virtual const uint32_t GetWidth() const = 0;
	virtual const uint32_t GetHeight() const = 0;

	virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
	virtual uint64_t GetDepthAttachmentRendererID() const = 0;

	static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
};
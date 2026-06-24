#pragma once

#include "Renderer/Framebuffer.h"
#include <glad/glad.h>

class OpenGLFramebuffer : public Framebuffer {
public:
	OpenGLFramebuffer(const FramebufferSpecification& spec);
	virtual ~OpenGLFramebuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void Resize(uint32_t width, uint32_t height) override;

	virtual Ref<Texture2D> GetColorAttachment(uint32_t index = 0) const override;
	virtual Ref<Texture2D> GetDepthAttachment() const override { return m_DepthAttachment; }

	virtual uint32_t GetHandle() const override { return m_Handle; }
	virtual uint32_t GetColorAttachmentCount() const override { return static_cast<uint32_t>(m_ColorAttachments.size()); }

	virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	virtual const uint32_t GetWidth() const { return m_Specification.Width; }
	virtual const uint32_t GetHeight() const { return m_Specification.Height; }
	
	virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override;
	virtual uint64_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment->GetHandle(); }
private:
	void Invalidate();
private:
	GLuint m_Handle = 0;

	FramebufferSpecification m_Specification;

	std::vector<Ref<Texture2D>> m_ColorAttachments;
	Ref<Texture2D> m_DepthAttachment;
};
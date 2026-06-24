#include "OpenGLFramebuffer.h"

#include "Core/Log/Log.h"

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
	: m_Specification(spec)
{
	Log::Trace("OpenGLFramebuffer - Creating the OpenGL Framebuffer");
	Invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
	Log::Trace("OpenGLFramebuffer - Deleting the OpenGL Framebuffer");
	glDeleteFramebuffers(1, &m_Handle);
}

void OpenGLFramebuffer::Invalidate() {
	Log::Trace("OpenGLFramebuffer::Invalidate - Invalidating the OpenGL Framebuffer");

	// If the Framebuffer exists, we delete it and its attachments
	if (m_Handle) {
		glDeleteFramebuffers(1, &m_Handle);
		m_ColorAttachments.clear();
		m_DepthAttachment = nullptr;
	}

	// Create the framebuffer
	glCreateFramebuffers(1, &m_Handle);

	// Create a color attachment for each specification in the vector
	if (!m_Specification.ColorAttachments.empty()) {
		m_ColorAttachments.resize(m_Specification.ColorAttachments.size());
		std::vector<GLenum> drawBuffers;
		drawBuffers.reserve(m_Specification.ColorAttachments.size());

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_Specification.ColorAttachments.size()); ++i) {
			auto& spec = m_Specification.ColorAttachments[i];
			spec.Width = m_Specification.Width;
			spec.Height = m_Specification.Height;
			m_ColorAttachments[i] = Texture2D::Create(spec);
			glNamedFramebufferTexture(m_Handle, GL_COLOR_ATTACHMENT0 + i, m_ColorAttachments[i]->GetHandle(), 0);
			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		glNamedFramebufferDrawBuffers(m_Handle, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	} else {
		glNamedFramebufferDrawBuffer(m_Handle, GL_NONE);
		glNamedFramebufferReadBuffer(m_Handle, GL_NONE);
	}

	// Create the Depth attachment only if there is one
	if (m_Specification.HasDepthAttachment) {
		m_Specification.DepthAttachmentSpecification.Width = m_Specification.Width;
		m_Specification.DepthAttachmentSpecification.Height = m_Specification.Height;
		m_DepthAttachment = Texture2D::Create(m_Specification.DepthAttachmentSpecification);
		glNamedFramebufferTexture(m_Handle, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAttachment->GetHandle(), 0);
	}

	// Check if the Framebuffer has been created successfully
	if (glCheckNamedFramebufferStatus(m_Handle, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Log::Error("OpenGLFramebuffer::Invalidate - Framebuffer is not complete");
	}
}

Ref<Texture2D> OpenGLFramebuffer::GetColorAttachment(uint32_t index) const {
	if (index < static_cast<uint32_t>(m_ColorAttachments.size())) {
		return m_ColorAttachments[index];
	}
	return nullptr;
}

uint64_t OpenGLFramebuffer::GetColorAttachmentRendererID(uint32_t index) const {
	if (index < static_cast<uint32_t>(m_ColorAttachments.size()) && m_ColorAttachments[index]) {
		return m_ColorAttachments[index]->GetHandle();
	}
	return 0;
}

void OpenGLFramebuffer::Bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
	glViewport(0, 0, m_Specification.Width, m_Specification.Height);
}

void OpenGLFramebuffer::Unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0) return;

	// Update the specification
	m_Specification.Width = width;
	m_Specification.Height = height;

	// We need to recreate the framebuffer
	Invalidate();
}
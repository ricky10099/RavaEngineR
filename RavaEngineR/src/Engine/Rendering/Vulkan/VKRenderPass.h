#pragma once

namespace VK {
class Swapchain;
class RenderPass {
   public:
	enum class Subpasses3D {
		GEOMETRY = 0,
		LIGHTING,
		TRANSPARENCY,
		NUMBER_OF_SUBPASSES,
	};

	enum class RenderTargets3D {
		ATTACHMENT_COLOR = 0,
		ATTACHMENT_DEPTH,
		// ATTACHMENT_GBUFFER_POSITION,
		// ATTACHMENT_GBUFFER_NORMAL,
		// ATTACHMENT_GBUFFER_COLOR,
		// ATTACHMENT_GBUFFER_MATERIAL,
		// ATTACHMENT_GBUFFER_EMISSION,
		NUMBER_OF_ATTACHMENTS,
	};

	enum class SubpassesPostProcessing {
		SUBPASS_BLOOM = 0,
		NUMBER_OF_SUBPASSES,
	};

	enum class RenderTargetsPostProcessing {
		ATTACHMENT_COLOR = 0,
		INPUT_ATTACHMENT_3DPASS_COLOR,
		INPUT_ATTACHMENT_GBUFFER_EMISSION,
		NUMBER_OF_ATTACHMENTS,
	};

	enum class SubpassesGUI {
		SUBPASS_GUI = 0,
		NUMBER_OF_SUBPASSES,
	};

	enum class RenderTargetsGUI {
		ATTACHMENT_COLOR = 0,
		NUMBER_OF_ATTACHMENTS,
	};

	public:
	RenderPass(Swapchain* swapChain);
	~RenderPass();

	NO_COPY(RenderPass)

		VkImageView GetImageViewColorAttachment() const { return _colorAttachmentView; }
	// VkImageView GetImageViewGBufferPosition() { return m_GBufferPositionView; }
	// VkImageView GetImageViewGBufferNormal() { return m_GBufferNormalView; }
	// VkImageView GetImageViewGBufferColor() { return m_GBufferColorView; }
	// VkImageView GetImageViewGBufferMaterial() { return m_GBufferMaterialView; }
	// VkImageView GetImageViewGBufferEmission() { return m_GBufferEmissionView; }
	// VkImage GetImageEmission() const { return m_GBufferEmissionImage; }
	// VkFormat GetFormatEmission() const { return m_bufferEmissionFormat; }

	VkFramebuffer Get3DFrameBuffer(int index) { return _3DFramebuffers[index]; }
	VkFramebuffer GetGUIFrameBuffer(int index) { return _GUIFramebuffers[index]; }
	// VkFramebuffer GetPostProcessingFrameBuffer(int index) { return m_PostProcessingFramebuffers[index]; }

	VkRenderPass Get3DRenderPass() const { return _3DRenderPass; }
	VkRenderPass GetGUIRenderPass() const { return _GUIRenderPass; }
	// VkRenderPass GetPostProcessingRenderPass() { return m_PostProcessingRenderPass; }

   private:
	Swapchain* _swapchain;
	VkExtent2D _renderPassExtent;

	VkFormat _depthFormat;
	// VkFormat m_bufferPositionFormat;
	// VkFormat m_bufferNormalFormat;
	// VkFormat m_bufferColorFormat;
	// VkFormat m_bufferMaterialFormat;
	// VkFormat m_bufferEmissionFormat;

	VkImage _depthImage;
	VkImage _colorAttachmentImage;
	// VkImage m_GBufferPositionImage;
	// VkImage m_GBufferNormalImage;
	// VkImage m_GBufferColorImage;
	// VkImage m_GBufferMaterialImage;
	// VkImage m_GBufferEmissionImage;

	VkImageView _depthImageView;
	VkImageView _colorAttachmentView;
	// VkImageView m_GBufferPositionView;
	// VkImageView m_GBufferNormalView;
	// VkImageView m_GBufferColorView;
	// VkImageView m_GBufferMaterialView;
	// VkImageView m_GBufferEmissionView;

	VkDeviceMemory _depthImageMemory;
	VkDeviceMemory _colorAttachmentImageMemory;
	// VkDeviceMemory m_GBufferPositionImageMemory;
	// VkDeviceMemory m_GBufferNormalImageMemory;
	// VkDeviceMemory m_GBufferColorImageMemory;
	// VkDeviceMemory m_GBufferMaterialImageMemory;
	// VkDeviceMemory m_GBufferEmissionImageMemory;

	std::vector<VkFramebuffer> _3DFramebuffers;
	std::vector<VkFramebuffer> _GUIFramebuffers;
	// std::vector<VkFramebuffer> m_postProcessingFramebuffers;

	VkRenderPass _3DRenderPass;
	VkRenderPass _GUIRenderPass;
	// VkRenderPass m_postProcessingRenderPass;

   private:
	void CreateColorAttachmentResources();
	void CreateDepthResources();

	void Create3DRenderPass();
	void CreateGUIRenderPass();
	// void CreatePostProcessingRenderPass();

	void Create3DFramebuffers();
	// void CreatePostProcessingFramebuffers();
	void CreateGUIFramebuffers();

	// void CreateGBufferImages();
	// void CreateGBufferImageViews();
	// void DestroyGBuffers();
};
}  // namespace VK
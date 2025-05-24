#include "rvpch.h"

#include "Engine/Vulkan/VKRenderer.h"
#include "Engine/Vulkan/VKUtils.h"
#include "Engine/Vulkan/VKSwapchain.h"
#include "Engine/Vulkan/VKDescriptor.h"
#include "Engine/Vulkan/VKWindow.h"
#include "Engine/Vulkan/VKRenderPass.h"
#include "Engine/Engine.h"

// std::shared_ptr<VK::Texture> DefaultTexture;
// std::shared_ptr<VK::Buffer> DummyBuffer;

namespace VK {
Unique<DescriptorPool> Renderer::GlobalDescriptorPool;
Renderer::Renderer(Window* window)
	: _window{window} {
	// Init();
}

Renderer::~Renderer() {
	FreeCommandBuffers();
}

void Renderer::Init() {
	RecreateSwapChain();
	RecreateRenderpass();
	CreateCommandBuffers();

	for (u32 i = 0; i < _uniformBuffers.size(); i++) {
		_uniformBuffers[i] = std::make_unique<Buffer>(
			sizeof(GlobalUbo),
			1,  // u32 instanceCount
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			Context::Instance->GetProperties().limits.minUniformBufferOffsetAlignment
		);
		_uniformBuffers[i]->Map();
	}

	// create a global pool for desciptor sets
	static constexpr u32 POOL_SIZE = 10000;

	GlobalDescriptorPool = DescriptorPool::Builder()
							   .SetMaxSets(MAX_FRAMES_SYNC * POOL_SIZE)
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_SYNC * 50)
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_SYNC * 7500)
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, MAX_FRAMES_SYNC * 2450)
							   .Build();

	// u32 dummy     = 0xffffffff;
	// DummyBuffer = std::make_shared<Buffer>(sizeof(u32));
	// DummyBuffer->Map();
	// DummyBuffer->WriteToBuffer(&dummy);
	// DummyBuffer->Flush();

	// DefaultTexture = std::make_shared<Rava::Texture>(true);
	// DefaultTexture->Init("Assets/System/Images/Rava.png", Rava::Texture::USE_SRGB);

	Unique<DescriptorSetLayout> globalDescriptorSetLayout =
		DescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)  // projection, view , lights
			//.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)  // spritesheet
			//.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)  // font atlas
			//.AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.Build();
	_globalDescriptorSetLayout = globalDescriptorSetLayout->GetDescriptorSetLayout();

	Unique<DescriptorSetLayout> pbrMaterialDescriptorSetLayout =
		DescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// diffuse color map
			.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// normal map
			.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// roughness metallic map
			.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// emissive map
			.AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// roughness map
			.AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			// metallic map
			.AddBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();

	Unique<DescriptorSetLayout> animationDescriptorSetLayout =
		DescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)  // shader data for animation
			.Build();

	std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDefaultDiffuse = {_globalDescriptorSetLayout};

	std::vector<VkDescriptorSetLayout> descriptorSetLayoutsPBR = {
		_globalDescriptorSetLayout, pbrMaterialDescriptorSetLayout->GetDescriptorSetLayout()
	};
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutsAnimation = {
		_globalDescriptorSetLayout,
		pbrMaterialDescriptorSetLayout->GetDescriptorSetLayout(),
		animationDescriptorSetLayout->GetDescriptorSetLayout()
	};

	for (u32 i = 0; i < MAX_FRAMES_SYNC; i++) {
		VkDescriptorBufferInfo bufferInfo = _uniformBuffers[i]->DescriptorInfo();
		DescriptorWriter(*globalDescriptorSetLayout, *GlobalDescriptorPool)
			.WriteBuffer(0, &bufferInfo)
			//.WriteImage(1, imageInfo0)
			//.WriteImage(2, imageInfo1)
			.Build(_globalDescriptorSets[i]);
	}

	// m_entityRenderSystem = std::make_unique<EntityRenderSystem>(_renderPass->Get3DRenderPass(), descriptorSetLayoutsAnimation);
	// m_entityAnimationRenderSystem =
	//	std::make_unique<EntityAnimationRenderSystem>(_renderPass->Get3DRenderPass(), descriptorSetLayoutsAnimation);
	// m_pointLightRenderSystem =
	//	std::make_unique<PointLightRenderSystem>(_renderPass->Get3DRenderPass(), _globalDescriptorSetLayout);

	//// m_Imgui = Imgui::Create(m_RenderPass->GetGUIRenderPass(), static_cast<u32>(m_SwapChain->ImageCount()));
	// m_editor = std::make_unique<Rava::Editor>(_renderPass->GetGUIRenderPass(), static_cast<u32>(_swapchain->ImageCount()));
	// for (u32 i = 0; i < _swapchain->ImageCount(); ++i) {
	//	m_editor->RecreateDescriptorSet(_swapchain->GetImageView(i), i);
	// }
}

void Renderer::RecreateSwapChain() {
	auto extent = _window->GetExtent();
	while (extent.width == 0 || extent.height == 0) {
		extent = _window->GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(Context::Instance->GetLogicalDevice());

	if (_swapchain == nullptr) {
		_swapchain = std::make_unique<Swapchain>(extent);
	} else {
		ENGINE_INFO("recreating swapchain at frame {0}", _frameCounter);
		std::shared_ptr<Swapchain> oldSwapChain = std::move(_swapchain);
		_swapchain                              = std::make_unique<Swapchain>(extent, oldSwapChain);
		if (!oldSwapChain->CompareFormats(*_swapchain.get())) {
			ENGINE_CRITICAL("swap chain image or depth format has changed");
		}
	}

	// if (m_editor) {
	//	for (u32 i = 0; i < _swapchain->ImageCount(); ++i) {
	//		m_editor->RecreateDescriptorSet(_swapchain->GetImageView(i), i);
	//	}
	// }
}

void Renderer::RecreateRenderpass() {
	_renderPass = std::make_shared<RenderPass>(_swapchain.get());
}

void Renderer::CreateCommandBuffers() {
	_commandBuffers.resize(MAX_FRAMES_SYNC);
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool        = Context::Instance->GetCommandPool();
	allocateInfo.commandBufferCount = static_cast<u32>(_commandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(Context::Instance->GetLogicalDevice(), &allocateInfo, _commandBuffers.data());
	VK_CHECK(result, "Failed to allocate Command Buffers!");
}

void Renderer::FreeCommandBuffers() {
	vkFreeCommandBuffers(
		Context::Instance->GetLogicalDevice(),
		Context::Instance->GetCommandPool(),
		static_cast<u32>(_commandBuffers.size()),
		_commandBuffers.data()
	);
	_commandBuffers.clear();
}

void Renderer::BeginFrame() {
	ENGINE_ASSERT(!_frameInProgress, "Can't Call BeginFrame while already in progress!");

	auto result = _swapchain->AcquireNextImage(&_currentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Recreate();
		// return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		ENGINE_CRITICAL("Failed to Acquire Swap Chain Image!");
	}

	_frameInProgress = true;
	_frameCounter++;

	auto commandBuffer = GetCurrentCommandBuffer();
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VK_CHECK(result, "Failed to Begin Recording Command Buffer!")

	_currentCommandBuffer = commandBuffer;
	// return commandBuffer;
	if (_currentCommandBuffer) {
		m_frameInfo = {
			_currentFrameIndex,
			0.0f, /* m_FrameTime */
			_currentCommandBuffer,

			_globalDescriptorSets[_currentFrameIndex]
		};
	}

	// m_editor->NewFrame();
}

// void Renderer::UpdateAnimations(entt::registry& registry) {
//	auto view = registry.view<Rava::Component::Model, Rava::Component::Transform, Rava::Component::Animation>();
//	for (auto entity : view) {
//		auto& mesh      = view.get<Rava::Component::Model>(entity);
//		auto& animation = view.get<Rava::Component::Animation>(entity);
//		auto skeleton   = mesh.model->GetSkeleton();
//		if (mesh.enable) {
//			animation.animationList->Update(*skeleton, _frameCounter);
//			mesh.model->UpdateAnimation(_frameCounter);
//		}
//	}
// }
//
// void Renderer::RenderpassEntities(entt::registry& registry, Rava::Camera& currentCamera) {
//	if (_currentCommandBuffer) {
//		GlobalUbo ubo{};
//		ubo.projection  = currentCamera.GetProjection();
//		ubo.view        = currentCamera.GetView();
//		ubo.inverseView = currentCamera.GetInverseView();
//		ubo.gamma       = Rava::Engine::s_Instance->GetGamma();
//		ubo.exposure    = Rava::Engine::s_Instance->GetExposure();
//		// ubo.Projection        = m_frameInfo.m_Camera->GetProjectionMatrix();
//		// ubo.View              = m_frameInfo.m_Camera->GetViewMatrix();
//		// ubo.AmbientLightColor = {1.0f, 1.0f, 1.0f, m_AmbientLightIntensity};
//		// m_LightSystem->Update(m_FrameInfo, ubo, registry);
//		// m_UniformBuffers[m_CurrentFrameIndex]->WriteToBuffer(&ubo);
//		// m_UniformBuffers[m_CurrentFrameIndex]->Flush();
//
//		// for (auto [entity, cam] : registry.view<Rava::Component::Camera>().each()) {
//		//	if (cam.currentCamera) {
//		//		ubo.projection  = cam.view.GetProjection();
//		//		ubo.view        = cam.view.GetView();
//		//		ubo.inverseView = cam.view.GetInverseView();
//		//	}
//		// }
//		m_pointLightRenderSystem->Update(m_frameInfo, ubo, registry);
//		_uniformBuffers[_currentFrameIndex]->WriteToBuffer(&ubo);
//		_uniformBuffers[_currentFrameIndex]->Flush();
//
//		Begin3DRenderPass(/*_currentCommandBuffer*/);
//		// BeginGUIRenderPass();
//	}
// }

void Renderer::RenderpassGUI() {
	if (_currentCommandBuffer) {
		EndRenderPass();  // end 3D renderpass
		// _swapchain->TransitionSwapChainImageLayout(
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		//	_currentImageIndex,
		//	_currentCommandBuffer
		//);
		BeginGUIRenderPass();
	}
}

void Renderer::EndFrame() {
	ENGINE_ASSERT(_frameInProgress, "Can't Call EndFrame while Frame is not in progress!");
	auto commandBuffer = GetCurrentCommandBuffer();

	VkResult result = vkEndCommandBuffer(commandBuffer);
	VK_CHECK(result, "Failed to Record Command buffer!")

	result = _swapchain->SubmitCommandBuffers(&commandBuffer, &_currentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _window->IsWindowResized()) {
		_window->ResetWindowResizedFlag();
		Recreate();
	} else if (result != VK_SUCCESS) {
		ENGINE_CRITICAL("Failed to Present Swap Chain image!");
	}

	_frameInProgress   = false;
	_currentFrameIndex = (_currentFrameIndex + 1) % MAX_FRAMES_SYNC;
}

void Renderer::Begin3DRenderPass(/*VkCommandBuffer commandBuffer*/) {
	assert(_frameInProgress);
	// assert(commandBuffer == GetCurrentCommandBuffer());

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass  = _renderPass->Get3DRenderPass();
	renderPassInfo.framebuffer = _renderPass->Get3DFrameBuffer(_currentImageIndex);

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _swapchain->GetSwapChainExtent();

	std::array<VkClearValue, static_cast<u32>(RenderPass::RenderTargets3D::NUMBER_OF_ATTACHMENTS)> clearValues{};
	clearValues[0].color = {
		RV::Engine::Instance->ClearColor.r,
		RV::Engine::Instance->ClearColor.g,
		RV::Engine::Instance->ClearColor.b,
		RV::Engine::Instance->ClearColor.a
	};
	clearValues[1].depthStencil    = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
	renderPassInfo.pClearValues    = clearValues.data();

	// vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x        = 0.0f;
	viewport.y        = static_cast<float>(_swapchain->GetSwapChainExtent().height);
	viewport.width    = static_cast<float>(_swapchain->GetSwapChainExtent().width);
	viewport.height   = static_cast<float>(_swapchain->GetSwapChainExtent().height) * -1.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{
		{0, 0},
        _swapchain->GetSwapChainExtent()
	};
	vkCmdSetViewport(_currentCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_currentCommandBuffer, 0, 1, &scissor);
}

void Renderer::BeginGUIRenderPass(/*VkCommandBuffer commandBuffer*/) {
	assert(_frameInProgress);
	// assert(commandBuffer == GetCurrentCommandBuffer());

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass  = _renderPass->GetGUIRenderPass();
	renderPassInfo.framebuffer = _renderPass->GetGUIFrameBuffer(_currentImageIndex);

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _swapchain->GetSwapChainExtent();

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = {
		{0.01f, 0.01f, 0.01f, 1.0f}
	};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues    = clearValues.data();

	vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x        = 0.0f;
	viewport.y        = 0.0f;
	viewport.width    = static_cast<float>(_swapchain->GetSwapChainExtent().width);
	viewport.height   = static_cast<float>(_swapchain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{
		{0, 0},
        _swapchain->GetSwapChainExtent()
	};
	vkCmdSetViewport(_currentCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_currentCommandBuffer, 0, 1, &scissor);
}

void Renderer::EndRenderPass(/*VkCommandBuffer commandBuffer*/) {
	assert(_frameInProgress);
	// assert(commandBuffer == GetCurrentCommandBuffer());

	// vkCmdEndRenderPass(commandBuffer);
	vkCmdEndRenderPass(_currentCommandBuffer);
}

// void Renderer::ResetEditor() {
//	m_editor->Reset();
// }
//
// void Renderer::UpdateEditor(Rava::Scene* scene) {
//	m_editor->Organize(scene, _currentImageIndex);
//	m_editor->InputHandle();
// }
//
// void Renderer::RenderEntities(Rava::Scene* scene) {
//	if (_currentCommandBuffer) {
//		// UpdateTransformCache(scene, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
//
//		auto& registry = scene->GetRegistry();
//
//		// 3D objects
//		m_entityRenderSystem->Render(m_frameInfo, registry);
//		m_entityAnimationRenderSystem->Render(m_frameInfo, registry);
//		// m_RenderSystemPbrSA->RenderEntities(m_frameInfo, registry);
//		// m_RenderSystemGrass->RenderEntities(m_frameInfo, registry);
//	}
// }
//
// void Renderer::RenderEnv(entt::registry& registry) {
//	if (_currentCommandBuffer) {
//		m_pointLightRenderSystem->Render(m_frameInfo, registry);
//	}
// }

void Renderer::EndScene() {
	if (_currentCommandBuffer) {
		// _swapchain->TransitionSwapChainImageLayout(
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		//	_currentImageIndex,
		//	_currentCommandBuffer
		//);
		// m_editor->Render(_currentCommandBuffer);
		EndRenderPass(/*_currentCommandBuffer*/);  // end GUI render pass
		/*_swapchain->TransitionSwapChainImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, _currentImageIndex, _currentCommandBuffer
		);*/
		EndFrame();
		// _swapchain->TransitionSwapChainImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
}

void Renderer::Recreate() {
	RecreateSwapChain();
	RecreateRenderpass();
	// CreateLightingDescriptorSets();
	// CreateRenderSystemBloom();
	// CreatePostProcessingDescriptorSets();
}

int Renderer::GetFrameIndex() const {
	assert(_frameInProgress);
	return _currentFrameIndex;
}

VkCommandBuffer Renderer::GetCurrentCommandBuffer() const {
	assert(_frameInProgress);
	return _commandBuffers[_currentFrameIndex];
}
}  // namespace VK
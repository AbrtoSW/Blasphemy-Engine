#pragma once
#include "types/vk_types.h"
#include "util/vk_util.h"
#include <vector>

class VulkanBackend;

class Renderer {

public:

	Renderer(VulkanBackend& vulkanBackend) : vkBackend(vulkanBackend){};

	void initRenderer();
	
	void renderFrame();

	void createOffscreenTargets();

	void destroyOffscreenTargets();
	void cleanup();

private:

	VulkanBackend& vkBackend;

	VkExtent2D drawExtent{};

	AllocatedImage drawImage{};
	AllocatedImage depthImage{};

	VkRenderPass drawImageRenderPass{};
	VkRenderPass swapchainRenderPass{};

	std::vector<VkFramebuffer> swapchainFrameBuffers{};
	VkFramebuffer drawImageFrameBuffer{};

	DeletionQueue rendererDeletionQueue{};


	
	
	void initDescriptors();
	void createRenderpasses();
	void createFramebuffers();


	void enqueueRenderPassessForDeletion();
	void enqueueFramebuffersForDeletion();
	void createDrawImageRenderpass();
	void createSwapchainRenderpass();
	
	void createSwapchainFramebuffer();
	void createDrawImageFramebuffer();

	void initDrawImageRenderpass(VkCommandBuffer cmd);
	void initSwapchainRenderpass(VkCommandBuffer cmd, uint32_t imageIndex);

};
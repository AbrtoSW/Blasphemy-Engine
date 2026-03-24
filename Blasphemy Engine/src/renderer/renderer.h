#pragma once
#include "types/vk_types.h"
#include "util/vk_util.h"

class VulkanBackend;

class Renderer {

public:

	Renderer(VulkanBackend& vulkanBackend) : vkBackend(vulkanBackend){};

	void renderFrame();

	void createOffscreenTargets();

	void destroyOffscreenTargets();

	void create_draw_image_renderpass();


	void create_swapchain_renderpass();
private:

	VulkanBackend& vkBackend;

	VkExtent2D drawExtent{};

	AllocatedImage drawImage{};
	AllocatedImage depthImage{};

	VkRenderPass drawImageRenderPass;
	VkRenderPass swapchainRenderPass;

	DeletionQueue rendererDeletionQueue;
	
	
	void initDescriptors();


	




};
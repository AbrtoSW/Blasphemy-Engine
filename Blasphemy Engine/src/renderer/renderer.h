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


private:

	VulkanBackend& vkBackend;

	VkExtent2D drawExtent{};

	AllocatedImage drawImage{};
	AllocatedImage depthImage{};

	DeletionQueue mainDeletionQueue;
	
	
	void initDescriptors();


	




};
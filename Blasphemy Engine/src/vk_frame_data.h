#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "vk_util.h"

struct FrameData {

	int currentFrame;
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore;
	VkFence renderFence;
	DeletionQueue deletionQueue;
	//std::unique_ptr<DescriptorAllocatorGrowable> frameDescriptors;

	AllocatedBuffer cameraUBO;
	VkDescriptorSet cameraSet;
	void* cameraUBOMapped = nullptr;
};

constexpr unsigned int FRAME_OVERLAP = 3;
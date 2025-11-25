#pragma once

#include <memory>


#include "volk/volk.h"
#include "types/vk_types.h"
#include "util/vk_util.h"

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

constexpr unsigned int FRAME_OVERLAP = 2;

class FrameManager {
public:
	FrameManager(uint32_t frameCount);

	FrameData& currentFrame();
	FrameData& getFrame(uint32_t index);

	uint32_t getCurrentIndex() const;
	void advance();

private:
	std::vector<FrameData> frames;
	uint32_t currentIndex;
};
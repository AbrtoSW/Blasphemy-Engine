#pragma once

#include <vma/vk_mem_alloc.h>
#include <chrono>
#include "engine/vk_frame_data.h"

class Platform;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

struct GpuCapabilities {
	bool supportsSync2;
	uint32_t apiVersion = 0;
};

class VulkanEngine {

public:

	VulkanEngine(Platform& platform) 
		: platform(platform) {}

	void init();
	void run();
	void cleanup();

private:

	Platform& platform;

	bool isInitalized = { false };
	bool stopRendering = { false };
	bool useValidationLayers = true;

	VmaAllocator vmaAllocator = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	uint32_t graphicsQueueFamily = 0;

	TimePoint lastTime = {};
	TimePoint currentTime = {};
	float deltaTime = 0.0f;

	std::vector<const char*> extensions;
	GpuCapabilities gpuCapability{};

	int frameNumber;
	FrameData frames[FRAME_OVERLAP];
	FrameData& get_current_frame() { return frames[frameNumber % FRAME_OVERLAP]; }

	bool init_vulkan();
	bool init_commands();
	bool init_sync_structures();

	void update_timing();



};
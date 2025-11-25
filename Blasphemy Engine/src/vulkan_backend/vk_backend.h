#pragma once

#define VK_NO_PROTOTYPES
#include <vma/vk_mem_alloc.h>
#include <chrono>
#include "volk/volk.h"
#include "vulkan_backend/vk_frame_manager.h"
#include "vulkan_backend/vk_swapchain.h"

class Platform;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

struct GpuCapabilities {
	bool supportsSync2;
	uint32_t apiVersion = 0;
};

struct EnabledFeatures {
	bool vulkan11_shaderDrawParameters = false;
	bool vulkan12_bufferDeviceAddress = false;
	bool vulkan12_descriptorIndexing = false;
	bool vulkan12_drawIndirectCount = false;
	bool vulkan12_scalarBlockLayout = false;
	bool vulkan13_synchronization2 = false;

	bool core_samplerAnisotropy = false;
	bool core_fillModeNonSolid = false;
};

class VulkanBackend {

public:

	VulkanBackend(Platform& platform) 
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

	VulkanSwapchain swapchain;

	EnabledFeatures enabledFeatures = {};

	uint32_t requestedMajor = 1;
	uint32_t requestedMinor = 1;

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

	void initSwapchain();
	bool createSurface();
	bool pickPhysicalDevice();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool findQueueFamilies(VkPhysicalDevice device);
	bool queryFeatures(VkPhysicalDevice device);
	bool createLogicalDevice();
	bool queryDriverVersion();
	void determineRequestedVulkanVersion();
	void printEnabledFeatures();

};
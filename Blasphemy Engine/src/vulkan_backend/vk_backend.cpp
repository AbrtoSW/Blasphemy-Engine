//Internal Solutions files
#include "vulkan_backend/vk_backend.h"
#include "platform/platform.h"
#include "vulkan_backend/vk_backend_instance_builder.h"
#include "util/vk_helper.h"
#include "vulkan_backend/vk_frame_manager.h"

//External Files
#include <iostream>
#include <volk/volk.h>


bool VulkanBackend::init() {

	if (isInitalized) {
		return true;
	}

	if (!initVulkan()) {
		return false;
	}
	
	lastTime = Clock::now();
	currentTime = lastTime;
	deltaTime = 0.0f;
	isInitalized = true;

	return true;
}


void VulkanBackend::cleanup() {

	if (!isInitalized) {
		return;
	}

	vkDeviceWaitIdle(device);

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {

		FrameData& frame = frameManager.getFrame(i);

		std::cout << "[Destroy] CommandPool for frame " << i << "\n";
		vkDestroyCommandPool(device, frame.commandPool, nullptr);

		std::cout << "[Destroy] RenderFence for frame " << i << "\n";
		vkDestroyFence(device, frame.renderFence, nullptr);

		std::cout << "[Destroy] SwapchainSemaphore for frame " << i << "\n";
		vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);
	}

	vkDestroyCommandPool(device, immediateCommandPool, nullptr);
	vkDestroyFence(device, immediateFence, nullptr);



	std::cout << "[Destroy] Swapchain images/views/framebuffers\n";
	vkSwapchain.destroy(device);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
	vkDestroyInstance(instance, nullptr);
	isInitalized = false;
}

bool VulkanBackend::initVulkan() {

	volkInitialize();

	VulkanInstanceBuilder builder(platform);

	InstanceBuildResult result = builder.create(useValidationLayers = true, instance, debug_messenger);

	if (!result.success)
		return false;

	auto selectedApiMajor = result.apiMajor;
	auto selectedApiMinor = result.apiMinor;

	std::cout << "Using Vulkan " << selectedApiMajor << "." << selectedApiMinor << "\n";

	volkLoadInstance(instance);

	if (!createSurface())
		return false;

	if (!queryDriverVersion())
		return false;

	if (!pickPhysicalDevice())
		return false;

	if (!createLogicalDevice())
		return false;

	volkLoadDevice(device);

	printAvailableGPUFeatures();

	createVMAAllocator();

	initSwapchain();
	
	initCommandPools();

	initFrameSyncObjects();

	return true;
}

bool VulkanBackend::queryDriverVersion() {

	uint32_t systemVersion = 0;

	if (vkEnumerateInstanceVersion(&systemVersion) != VK_SUCCESS)
		return false;

	gpuCapability.apiVersion = systemVersion;

	uint32_t sysMajor = VK_VERSION_MAJOR(systemVersion);
	uint32_t sysMinor = VK_VERSION_MINOR(systemVersion);

	std::cout << "Vulkan Driver Vulkan version: "
		<< sysMajor << "." << sysMinor << "\n";

	// Minimum required: 1.1
	if (sysMajor < 1 || (sysMajor == 1 && sysMinor < 1)) {
		std::cout << "Vulkan Error: Minimum required Vulkan version is 1.1\n";
		return false;
	}

	return true;
}

bool VulkanBackend::initCommandPools() {

	std::cout << "Initiating vulkan command pools\n";

	VkCommandPoolCreateInfo commandPoolInfo = vkhelper::command_pool_create_info(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {

		FrameData& frame = frameManager.getFrame(i);
		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frame.commandPool));

		VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::command_buffer_allocate_info(frame.commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frame.mainCommandBuffer));
	}

	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &immediateCommandPool));

	VkCommandBufferAllocateInfo cmdAllocInfo = vkhelper::command_buffer_allocate_info(immediateCommandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &immediateCommandBuffer));


	return true;
}

bool VulkanBackend::initFrameSyncObjects() {

	std::cout << "Initiating vulkan sync structures \n";
	VkFenceCreateInfo fenceInfo = vkhelper::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreInfo = vkhelper::semaphore_create_info();

	for (unsigned int i = 0; i < FRAME_OVERLAP; ++i) {

		FrameData& frame = frameManager.getFrame(i);

		VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frame.renderFence));

		VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.swapchainSemaphore));
	}

	VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &immediateFence));
	
	return true;
}

bool VulkanBackend::createVMAAllocator() {

	VmaVulkanFunctions funcs{};
	funcs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	funcs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

	funcs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	funcs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	funcs.vkAllocateMemory = vkAllocateMemory;
	funcs.vkFreeMemory = vkFreeMemory;
	funcs.vkMapMemory = vkMapMemory;
	funcs.vkUnmapMemory = vkUnmapMemory;
	funcs.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	funcs.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	funcs.vkBindBufferMemory = vkBindBufferMemory;
	funcs.vkBindImageMemory = vkBindImageMemory;
	funcs.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	funcs.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	funcs.vkCreateBuffer = vkCreateBuffer;
	funcs.vkDestroyBuffer = vkDestroyBuffer;
	funcs.vkCreateImage = vkCreateImage;
	funcs.vkDestroyImage = vkDestroyImage;
	funcs.vkCmdCopyBuffer = vkCmdCopyBuffer;

#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
	funcs.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	funcs.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif

#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
	funcs.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	funcs.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
#endif

#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
	funcs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
#endif

	VmaAllocatorCreateInfo info{};
	info.instance = instance;
	info.physicalDevice = physicalDevice;
	info.device = device;
	info.pVulkanFunctions = &funcs;
	info.vulkanApiVersion = VK_API_VERSION_1_1;
	info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	if (vmaCreateAllocator(&info, &vmaAllocator) != VK_SUCCESS) {
		std::cout << "VMA allocator creation failed\n";
		return false;
	}

	return true;
}

void VulkanBackend::update_timing() {
	currentTime = Clock::now();
	deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;
}

void VulkanBackend::initSwapchain() {
	VkExtent2D e = platform.getWindowExtent();
	vkSwapchain.create(physicalDevice, device, surface, e.width, e.height);

	//renderer->framebuffer_image_resources
}

void VulkanBackend::printAvailableGPUFeatures() {
	std::cout << "\n=== GPU FEATURE REPORT ===\n";

	std::cout << "Core Sampler Anisotropy       : "
		<< (enabledFeatures.core_samplerAnisotropy ? "YES" : "NO") << "\n";

	std::cout << "Core FillModeNonSolid         : "
		<< (enabledFeatures.core_fillModeNonSolid ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.1 ShaderDrawParams   : "
		<< (enabledFeatures.vulkan11_shaderDrawParameters ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.2 BufferDeviceAddress: "
		<< (enabledFeatures.vulkan12_bufferDeviceAddress ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.2 DescriptorIndexing : "
		<< (enabledFeatures.vulkan12_descriptorIndexing ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.2 DrawIndirectCount  : "
		<< (enabledFeatures.vulkan12_drawIndirectCount ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.2 ScalarBlockLayout  : "
		<< (enabledFeatures.vulkan12_scalarBlockLayout ? "YES" : "NO") << "\n";

	std::cout << "Vulkan 1.3 Synchronization2   : "
		<< (enabledFeatures.vulkan13_synchronization2 ? "YES" : "NO") << "\n";

	std::cout << "===========================\n\n";
}

FrameData& VulkanBackend::getCurrentFrame() {
	return frameManager.currentFrame();
}

void VulkanBackend::advanceFrame() {
	frameManager.nextFrame();
}
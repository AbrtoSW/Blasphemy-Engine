#define VMA_IMPLEMENTATION

//Internal Solutions files
#include "engine/vk_engine.h"
#include "platform/platform.h"

//External Files
#include <vkbootstrap/VkBootstrap.h>
#include <iostream>
#include <SDL3/SDL.h>

void VulkanEngine::init() {
	if (isInitalized) {
		std::cout << "Engine init() called twice.\n";
		return;
	}

	init_vulkan();
	

	frameNumber = 0;
	lastTime = Clock::now();
	currentTime = lastTime;
	deltaTime = 0.0f;
	isInitalized = true;
}

void VulkanEngine::run() {
	if (!isInitalized) {
		std::cout << "Engine run() called before init().\n";
		return;
	}

	std::cout << "Engine Entering main loop.\n";

	while (!stopRendering && !platform.shouldClose()) {
		// 1) timing
		update_timing();

		// 2) poll platform events (SDL etc. lives in Platform)
		platform.pollEvents();

		// 3) per-frame engine work will go here later:
		//    - acquire swapchain image
		//    - get current FrameData
		//    - record command buffers
		//    - submit + present

		frameNumber++;
	}

	std::cout << "Engine Exiting main loop.\n";
}


bool VulkanEngine::init_vulkan() {
	std::cout << "Engine init_vulkan() skeleton.\n";

	extensions.clear();
	platform.getRequiredVulkanExtensions(extensions);

	uint32_t systemVersion = 0;
	if (vkEnumerateInstanceVersion(&systemVersion) != VK_SUCCESS) {
		std::cout << "Vulkan Error : vkEnumerateInstanceVersion failed\n";
		return false;
	}

	gpuCapability.apiVersion = systemVersion;

	uint32_t systemMajor = VK_VERSION_MAJOR(systemVersion);
	uint32_t systemMinor = VK_VERSION_MINOR(systemVersion);
	uint32_t systemPatch = VK_VERSION_PATCH(systemVersion);
	std::cout << "Vulkan Driver Vulkan version: " << systemMajor << "." << systemMinor << "." << systemPatch << "\n";

	uint32_t reqMajor = 1;
	uint32_t reqMinor = 1;

	if (systemMajor < 1 || (systemMajor == 1 && systemMinor < 1)) {
		std::cout << "Vulkan Error : minimum version required is vulkan 1.1. Driver supports only" << systemMajor << "." << systemMinor;
		return false;
	}

	if (systemMajor > 1 || (systemMajor == 1 && systemMinor >= 3)) {
		reqMajor = 1; reqMinor = 3;
	}
	else if (systemMajor > 1 || (systemMajor == 1 && systemMinor >= 2)) {
		reqMajor = 1; reqMinor = 2;
	}
	else {
		reqMajor = 1; reqMinor = 1;
	}

	std::cout << "Vulkan Requesting Vulkan " << reqMajor << "." << reqMinor << "\n";

	vkb::InstanceBuilder builder;

	auto instanceResult = builder
		.set_app_name("Blasphemy Engine")
		.request_validation_layers(useValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(reqMajor, reqMinor, 0)
		.enable_extensions(extensions)
		.build();
	
	if (!instanceResult) {
		std::cout << "[Vulkan] ERROR: Instance creation failed: " << instanceResult.error().message() << "\n";
		return false;
	}

	vkb::Instance vkbInstance = instanceResult.value();

	instance = vkbInstance.instance;
	debug_messenger = vkbInstance.debug_messenger;

	surface = platform.createVulkanSurface(instance, nullptr);
	if (surface == VK_NULL_HANDLE) {
		std::cout << "Vulkan Error: Failed to create vulkan surface\n";
		return false;
	}

	VkPhysicalDeviceVulkan13Features feats13 = {};
	feats13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	feats13.synchronization2 = VK_TRUE;


	VkPhysicalDeviceVulkan12Features feats12 = {};
	feats12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	feats12.bufferDeviceAddress = VK_TRUE;
	feats12.descriptorIndexing = VK_TRUE;
	feats12.drawIndirectCount = VK_TRUE;

	VkPhysicalDeviceScalarBlockLayoutFeatures sbl{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES };
	sbl.scalarBlockLayout = VK_TRUE;
	feats12.pNext = &sbl;

	VkPhysicalDeviceVulkan11Features feats11 = {};
	feats11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	feats11.shaderDrawParameters = VK_TRUE;

	VkPhysicalDeviceFeatures core = {};
	core.samplerAnisotropy = VK_TRUE;
	core.fillModeNonSolid = VK_TRUE;

	vkb::PhysicalDeviceSelector selector(vkbInstance);
	selector.set_surface(surface)
	.set_required_features(core)
	.set_minimum_version(reqMajor, reqMinor);

	// only require 1.2+ features if we are really requesting 1.2+
	if (reqMajor == 1 && reqMinor >= 3) {
		selector.set_required_features_13(feats13);
		selector.set_required_features_12(feats12);
		selector.set_required_features_11(feats11);
	}

	// If requesting Vulkan 1.2 (but not 1.3)
	else if (reqMajor == 1 && reqMinor == 2) {
		selector.set_required_features_12(feats12);
		selector.set_required_features_11(feats11);
	}

	// If requesting Vulkan 1.1
	else if (reqMajor == 1 && reqMinor == 1) {
		selector.set_required_features_11(feats11);
	}


	auto physicalResult = selector.select();
	if (!physicalResult) {
		std::cout << "Vulkan Error: Physical device selection failed, " << physicalResult.error().message() << "\n";
	}

	vkb::PhysicalDevice chosen = physicalResult.value();
	physicalDevice = chosen.physical_device;
	
	gpuCapability.supportsSync2 = false;

	VkPhysicalDeviceSynchronization2Features sync2{};
	sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;


	VkPhysicalDeviceFeatures2 feat2{};
	feat2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	feat2.pNext = &sync2;

	vkGetPhysicalDeviceFeatures2(physicalDevice, &feat2);

	if (sync2.synchronization2 == VK_TRUE) {
		gpuCapability.supportsSync2 = true;
	}

	std::cout << "Vulkan Sync2 support: " << (gpuCapability.supportsSync2 ? "YES" : "NO") << "\n";
	
	vkb::DeviceBuilder deviceBuilder(chosen);

	auto deviceResult = deviceBuilder.build();

	if (!deviceResult){
		std::cout << "Vulkan ERROR: Logical device creation failed: " << deviceResult.error().message() << "\n";
		return false;
	}

	vkb::Device vkbDevice = deviceResult.value();
	device = vkbDevice.device;

	auto graphicsQueueResult = vkbDevice.get_queue(vkb::QueueType::graphics);
	auto graphicsQueueFamilyResult = vkbDevice.get_queue_index(vkb::QueueType::graphics);

	if (!graphicsQueueResult || !graphicsQueueFamilyResult) {
		std::cout << "Vulkan ERROR: Failed to get graphics queue/index.\n";
		return false;
	}

	graphicsQueueFamily = graphicsQueueFamilyResult.value();
	graphicsQueue = graphicsQueueResult.value();

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VkResult vmaResult = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	if (vmaResult != VK_SUCCESS) {
		std::cout << "Vulkan ERROR: vmaCreateAllocator failed: " << vmaResult << "\n";
		return false;
	}

	return true;
}

bool VulkanEngine::init_commands() {
	std::cout << "[Engine] init_commands() skeleton.\n";
	// TODO: create command pools / buffers (immediate + per-frame)
	return true;
}

bool VulkanEngine::init_sync_structures() {
	std::cout << "[Engine] init_sync_structures() skeleton.\n";
	// TODO: create fences/semaphores for frames[FRAME_OVERLAP]
	return true;
}


void VulkanEngine::update_timing() {
	currentTime = Clock::now();
	deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;
}
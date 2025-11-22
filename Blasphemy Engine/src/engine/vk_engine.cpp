#define VMA_IMPLEMENTATION
//Internal Solutions files

#include "engine/vk_engine.h"
#include "platform/platform.h"
#include "engine/vk_engine_instance_builder.h"

//External Files
#include <iostream>

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

	volkInitialize();

	VulkanInstanceBuilder builder(platform);

	if (!builder.create(useValidationLayers, instance, debug_messenger)) {
		return false;
	}

	volkLoadInstance(instance);
	
	// 2. surface
	if (!createSurface())
		return false;

	// 3. Vulkan driver version
	if (!queryDriverVersion())
		return false;

	// 4. dynamic Vulkan version selection (1.1 / 1.2 / 1.3)
	determineRequestedVulkanVersion();

	// 5. GPU select
	if (!pickPhysicalDevice())
		return false;

	// 6. logical device
	if (!createLogicalDevice())
		return false;

	volkLoadDevice(device);

	printEnabledFeatures();

	initSwapchain();
	
	return true;
}

bool VulkanEngine::queryDriverVersion() {

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

void VulkanEngine::determineRequestedVulkanVersion() {

	uint32_t sysMajor = VK_VERSION_MAJOR(gpuCapability.apiVersion);
	uint32_t sysMinor = VK_VERSION_MINOR(gpuCapability.apiVersion);

	requestedMajor = 1;
	requestedMinor = 1;

	if (sysMajor > 1 || (sysMajor == 1 && sysMinor >= 3)) {
		requestedMajor = 1;
		requestedMinor = 3;
	}
	else if (sysMajor > 1 || (sysMajor == 1 && sysMinor >= 2)) {
		requestedMajor = 1;
		requestedMinor = 2;
	}
	else {
		requestedMajor = 1;
		requestedMinor = 1;
	}

	std::cout << "Vulkan Requesting Vulkan "
		<< requestedMajor << "." << requestedMinor << "\n";
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

void VulkanEngine::initSwapchain() {
	VkExtent2D e = platform.getWindowExtent();
	swapchain.create(physicalDevice, device, surface, e.width, e.height);

	//renderer->framebuffer_image_resources
}



void VulkanEngine::printEnabledFeatures() {
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
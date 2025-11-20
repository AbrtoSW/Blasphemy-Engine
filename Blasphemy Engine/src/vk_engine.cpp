#define VMA_IMPLEMENTATION

#include "vk_engine.h"
#include "platform.h"

#include <vkbootstrap/VkBootstrap.h>
#include <iostream>
#include <SDL3/SDL.h>

void VulkanEngine::init() {
	if (isInitalized) {
		std::cout << "Engine init() called twice.\n";
		return;
	}
	

	frameNumber = 0;
	lastTime = Clock::now();
	currentTime = lastTime;
	deltaTime = 0.0f;
	isInitalized = true;
}

void VulkanEngine::run() {
	if (!isInitalized) {
		std::cout << "[Engine] run() called before init().\n";
		return;
	}

	std::cout << "[Engine] Entering main loop.\n";

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

	std::cout << "[Engine] Exiting main loop.\n";
}


bool VulkanEngine::init_vulkan() {
	std::cout << "[Engine] init_vulkan() skeleton.\n";

	// TODO:
	//   - platform.getRequiredVulkanExtensions(extensions);
	//   - vkEnumerateInstanceVersion(&gpuCaps.apiVersion);
	//   - build instance via vkb::InstanceBuilder
	//   - surface = platform.createVulkanSurface(instance, nullptr);
	//   - select physical device via vkb::PhysicalDeviceSelector
	//   - create logical device via vkb::DeviceBuilder
	//   - get graphicsQueue + graphicsQueueFamily
	//   - create VMA allocator

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
#include "engine/vk_engine.h"
#include "platform/platform.h"
#include <iostream>

bool VulkanEngine::createSurface() {
	surface = platform.createVulkanSurface(instance, nullptr);
	if (surface == VK_NULL_HANDLE) {
		std::cout << "Failed to create SDL Vulkan surface\n";
		return false;
	}
	return true;
}
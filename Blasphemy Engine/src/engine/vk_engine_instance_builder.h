#pragma once
#include "volk/volk.h"
#include <vector>

class Platform;

class VulkanInstanceBuilder {
public:

	VulkanInstanceBuilder(Platform& platform) : platform(platform) {}
	bool create(bool enableValidation, VkInstance& outInstance, VkDebugUtilsMessengerEXT& debugMessenger);

private:
	Platform& platform;
	bool validation = false;
	
	bool createInstance(VkInstance& outInstance);
	bool createDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);
};
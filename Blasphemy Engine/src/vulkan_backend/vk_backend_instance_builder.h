#pragma once
#include "volk/volk.h"
#include <vector>

class Platform;

struct InstanceBuildResult {
	bool success;
	uint32_t apiMajor;
	uint32_t apiMinor;
};

class VulkanInstanceBuilder {
public:

	VulkanInstanceBuilder(Platform& platform) : platform(platform) {}
	InstanceBuildResult create(bool enableValidation, VkInstance& outInstance, VkDebugUtilsMessengerEXT& debugMessenger);

private:

	Platform& platform;
	bool validation = false;

	
	bool createInstance(VkInstance& instance, uint32_t reqMajor, uint32_t reqMinor);
	bool createDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);
	void filterLayers(std::vector<const char*>& layers);
};
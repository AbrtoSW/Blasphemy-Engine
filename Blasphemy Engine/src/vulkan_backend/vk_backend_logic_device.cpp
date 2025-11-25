#include "vulkan_backend/vk_backend.h"
#include <iostream>

bool VulkanBackend::createLogicalDevice() {
	
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = graphicsQueueFamily;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures core{};
	core.samplerAnisotropy = VK_TRUE;
	core.fillModeNonSolid = VK_TRUE;

	VkPhysicalDeviceVulkan13Features f13 = {};
	f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	f13.synchronization2 = gpuCapability.supportsSync2 ? VK_TRUE : VK_FALSE;

	VkPhysicalDeviceVulkan12Features f12 = {};
	f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	f12.bufferDeviceAddress = VK_TRUE;
	f12.descriptorIndexing = VK_TRUE;
	f12.drawIndirectCount = VK_TRUE;
	f12.pNext = &f13;

	VkPhysicalDeviceVulkan11Features f11 = {};
	f11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	f11.shaderDrawParameters = VK_TRUE;
	f11.pNext = &f12;

	VkPhysicalDeviceFeatures2 features2 = {};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features = core;
	features2.pNext = &f11;

	const char* deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &features2;

	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;

	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		std::cout << "Failed to create logical device\n";
		return false;
	}

	vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

	return true;
}
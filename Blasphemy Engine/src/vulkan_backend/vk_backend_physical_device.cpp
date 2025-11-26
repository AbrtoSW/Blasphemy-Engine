#include "vulkan_backend/vk_backend.h"
#include <iostream>

bool VulkanBackend::pickPhysicalDevice() {

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		std::cout << "No Vulkan compatible GPUs found\n";
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


	for (auto& dev : devices) {

		if (!findQueueFamilies(dev)) continue;
		if (!checkDeviceExtensionSupport(dev)) continue;
		if (!queryFeatures(dev)) continue;

		physicalDevice = dev;
		return true;
	}
	
	std::cout << "No suitable GPU found\n";
	
	return false;
}

bool VulkanBackend::checkDeviceExtensionSupport(VkPhysicalDevice dev) {

	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, nullptr);

	std::vector<VkExtensionProperties> available(extCount);
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, available.data());

	const char* requiredExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (auto& req : requiredExtensions) {
		bool found = false;
		for (auto& ext : available) {
			if (strcmp(req, ext.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}


bool VulkanBackend::findQueueFamilies(VkPhysicalDevice device) {

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> props(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, props.data());

	for (uint32_t i = 0; i < queueCount; i++) {
		bool graphics = props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

		VkBool32 presentSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);

		if (graphics && presentSupported) {
			graphicsQueueFamily = i;
			return true;
		}
	}

	return false;
}

bool VulkanBackend::queryFeatures(VkPhysicalDevice device) {
	// core
	VkPhysicalDeviceFeatures core{};
	vkGetPhysicalDeviceFeatures(device, &core);

	if (!core.samplerAnisotropy) return false;
	// fillModeNonSolid optional, skip if needed

	// Vulkan 1.1 / 1.2 / 1.3 feature chain
	VkPhysicalDeviceVulkan13Features f13{};
	f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

	VkPhysicalDeviceVulkan12Features f12{};
	f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	f12.pNext = &f13;

	VkPhysicalDeviceVulkan11Features f11{};
	f11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	f11.pNext = &f12;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &f11;

	vkGetPhysicalDeviceFeatures2(device, &features2);

	bool hasDrawParams = f11.shaderDrawParameters == VK_TRUE;
	bool hasIndexing = f12.descriptorIndexing == VK_TRUE;
	bool hasBDA = f12.bufferDeviceAddress == VK_TRUE;
	bool hasDICount = f12.drawIndirectCount == VK_TRUE;
	bool hasSync2 = f13.synchronization2 == VK_TRUE;

	if (!hasDrawParams)
		return false;

	if (hasIndexing && hasBDA && hasDICount && hasSync2)
		rendererMode = RendererMode::Modern;
	else
		rendererMode = RendererMode::Legacy;

	// store sync2 support
	gpuCapability.supportsSync2 = hasSync2;

	enabledFeatures.vulkan11_shaderDrawParameters = hasDrawParams;

	enabledFeatures.vulkan12_bufferDeviceAddress = hasBDA;
	enabledFeatures.vulkan12_descriptorIndexing = hasIndexing;
	enabledFeatures.vulkan12_drawIndirectCount = hasDICount;
	enabledFeatures.vulkan12_scalarBlockLayout = (f12.scalarBlockLayout == VK_TRUE);

	enabledFeatures.vulkan13_synchronization2 = hasSync2;

	return true;
}
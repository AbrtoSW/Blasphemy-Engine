#include "engine/vk_engine.h"
#include <iostream>

bool VulkanEngine::pickPhysicalDevice() {

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

bool VulkanEngine::checkDeviceExtensionSupport(VkPhysicalDevice dev) {

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


bool VulkanEngine::findQueueFamilies(VkPhysicalDevice device) {

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> props(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, props.data());

	for (int i = 0; i < queueCount; i++) {
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

bool VulkanEngine::queryFeatures(VkPhysicalDevice device) {
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

	//Check requested version feature requirements
	if (requestedMajor == 1 && requestedMinor >= 3) {
		if (!f13.synchronization2) return false;
		if (!f12.descriptorIndexing) return false;
		if (!f12.bufferDeviceAddress) return false;
		if (!f12.drawIndirectCount) return false;
		if (!f11.shaderDrawParameters) return false;
	}
	else if (requestedMajor == 1 && requestedMinor == 2) {
		if (!f12.descriptorIndexing) return false;
		if (!f12.bufferDeviceAddress) return false;
		if (!f12.drawIndirectCount) return false;
		if (!f11.shaderDrawParameters) return false;
	}
	else if (requestedMajor == 1 && requestedMinor == 1) {
		if (!f11.shaderDrawParameters) return false;
	}

	// store sync2 support
	gpuCapability.supportsSync2 = (f13.synchronization2 == VK_TRUE);

	enabledFeatures.vulkan11_shaderDrawParameters = f11.shaderDrawParameters;

	enabledFeatures.vulkan12_bufferDeviceAddress = f12.bufferDeviceAddress;
	enabledFeatures.vulkan12_descriptorIndexing = f12.descriptorIndexing;
	enabledFeatures.vulkan12_drawIndirectCount = f12.drawIndirectCount;
	enabledFeatures.vulkan12_scalarBlockLayout = f12.scalarBlockLayout;

	enabledFeatures.vulkan13_synchronization2 = f13.synchronization2;

	return true;
}
#include "engine/vk_engine_instance_builder.h"
#include "platform/platform.h"
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
	std::cout << data->pMessage << "\n";
	return VK_FALSE;
}

bool VulkanInstanceBuilder::create(bool enableValidation, VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) {
	validation = enableValidation;
	if (!createInstance(instance)) return false;
	if (validation && !createDebugMessenger(instance, debugMessenger)) return false;
	return true;
}

bool VulkanInstanceBuilder::createInstance(VkInstance& instance) {

	uint32_t systemVersion = 0;

	if (vkEnumerateInstanceVersion(&systemVersion) != VK_SUCCESS) {
		std::cout << "vkEnumerateInstanceVersion failed\n";
		return false;
	}

	uint32_t systemMajor = VK_VERSION_MAJOR(systemVersion);
	uint32_t systemMinor = VK_VERSION_MINOR(systemVersion);

	if (systemMajor < 1 || (systemMajor == 1 && systemMinor < 1)) {
		std::cout << "Minimum supported Vulkan version is 1.1, driver reports " << systemMajor << "." << systemMinor << "\n";
		return false;
	}

	uint32_t reqMajor = 1;
	uint32_t reqMinor = 1;

	if (systemMinor >= 3) {
		reqMinor = 3;
	} else if (systemMinor >= 2) {
		reqMinor = 2;
	} else {
		reqMinor = 1;
	}

	std::cout << "Requesting Vulkan " << reqMajor << "." << reqMinor << "\n";

	//any extensions required from SDL
	std::vector<const char*> extensions;

	platform.getRequiredVulkanExtensions(extensions);

	//add debug utils if validation triggers
	if (validation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	//Validation layers
	std::vector<const char*> layers;
	if (validation) {
		layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "Blasphemy Engine";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName = "Blasphemy";
	app.engineVersion = VK_MAKE_VERSION(1,0,0);
	app.apiVersion = VK_MAKE_API_VERSION(0, reqMajor, reqMinor, 0);


	VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
	if (validation) {
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = debugCallback;
	}

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &app;
	instanceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
	instanceCreateInfo.ppEnabledLayerNames = layers.data();
	instanceCreateInfo.pNext = validation ? (void*)&debugInfo : nullptr;

	if (vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance) != VK_SUCCESS) {
		std::cout << "Failed to create vulkan instance\n";
		return false;
	}

	return true;
}

bool VulkanInstanceBuilder::createDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) {

	if (!validation) return true;

	VkDebugUtilsMessengerCreateInfoEXT info{};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = debugCallback;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (!func) return false;

	return func(instance, &info, nullptr, &debugMessenger) == VK_SUCCESS;
}

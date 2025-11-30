#include "vulkan_backend/vk_backend_instance_builder.h"
#include "platform/platform.h"
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
	std::cout << data->pMessage << "\n";
	return VK_FALSE;
}

void VulkanInstanceBuilder::filterLayers(std::vector<const char*>& layers) {

	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> props(count);
	vkEnumerateInstanceLayerProperties(&count, props.data());

	bool foundValidation = false;

	for (auto& p : props) {
		if (strcmp(p.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
			foundValidation = true;
			break;
		}
	}
	
	if (!foundValidation) {
		std::cout << "[Validation] VK_LAYER_KHRONOS_validation not found. Validation is DISABLED.\n";
		layers.clear();               // no layer available
		return;
	}

	layers.clear();
	layers.push_back("VK_LAYER_KHRONOS_validation");
}


InstanceBuildResult VulkanInstanceBuilder::create(bool enableValidation, VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) {

	InstanceBuildResult r{};
	r.success = false;

	validation = enableValidation;

	uint32_t reqMajor = 1;
	uint32_t reqMinor = 1;

	if (!createInstance(instance, reqMajor, reqMinor))
		return r;

	if (validation && !createDebugMessenger(instance, debugMessenger))
		return r;

	r.success = true;
	r.apiMajor = reqMajor;
	r.apiMinor = reqMinor;

	return r;
}

bool VulkanInstanceBuilder::createInstance(VkInstance& instance, uint32_t reqMajor, uint32_t reqMinor) {
	// Get SDL extensions
	std::vector<const char*> extensions;
	platform.getRequiredVulkanExtensions(extensions);

#ifdef _DEBUG
	if (validation)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	std::vector<const char*> layers;
	if (validation)
		layers.push_back("VK_LAYER_KHRONOS_validation");

	filterLayers(layers);

	VkApplicationInfo app{};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "Blasphemy Engine";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName = "Blasphemy";
	app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion = VK_MAKE_API_VERSION(0,reqMajor, reqMinor, 0);

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

	VkValidationFeatureEnableEXT enables[] = {
	   VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
	   VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
	};

	VkValidationFeaturesEXT validationFeatures{};
	if (validation) {
		validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		validationFeatures.enabledValidationFeatureCount = 2;
		validationFeatures.pEnabledValidationFeatures = enables;
		validationFeatures.pNext = nullptr;
	}

	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app;
	info.enabledExtensionCount = (uint32_t)extensions.size();
	info.ppEnabledExtensionNames = extensions.data();
	info.enabledLayerCount = (uint32_t)layers.size();
	info.ppEnabledLayerNames = layers.data();
	info.pNext = validation ? (void*)&validationFeatures : nullptr;

	if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS) {
		std::cout << "Failed to create Vulkan instance\n";
		return false;
	}

	return true;
}

bool VulkanInstanceBuilder::createDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) {

	if (!validation) return true;

	VkDebugUtilsMessengerCreateInfoEXT info = {};
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

	VkResult r = func(instance, &info, nullptr, &debugMessenger);
	if (r == VK_SUCCESS) {
		std::cout << "Vulkan Debug: ENABLED\n";
		return true;
	}

	return false;
}
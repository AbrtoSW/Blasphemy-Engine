#include "vulkan_backend/vk_swapchain.h"
#include "util/vk_helper.h"
#include "util/vk_util.h"
#include <algorithm>
#include <iostream>

bool VulkanSwapchain::create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height) {

	VkSurfaceCapabilitiesKHR caps{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

	uint32_t presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, nullptr);
	std::vector<VkPresentModeKHR> presents(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, presents.data());

	VkSurfaceFormatKHR chosenFormat = chooseSurfaceFormat(formats);
	VkPresentModeKHR chosenPresent = choosePresentMode(presents);
	VkExtent2D chosenExtent = chooseExtent(caps, width, height);

	imageFormat = chosenFormat.format;
	extent = chosenExtent;

	uint32_t desiredImageCount = caps.minImageCount + 1;

	if (caps.maxImageCount > 0 && desiredImageCount > caps.maxImageCount) {
		desiredImageCount = caps.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = desiredImageCount;
	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.imageExtent = chosenExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = caps.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chosenPresent;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		std::cout << "Failed to create swapchain\n";
		return false;
	}

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

	views.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo viewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewCI.image = images[i];
		viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCI.format = chosenFormat.format;
		viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.subresourceRange.levelCount = 1;
		viewCI.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewCI, nullptr, &views[i]) != VK_SUCCESS) {
			std::cout << "Failed to create swapchain image view\n";
			return false;
		}
	}

	renderSemaphores.resize(imageCount);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkhelper::semaphore_create_info();
	for (size_t i = 0; i < imageCount; i++) {
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]));
	}

	return true;
}

void VulkanSwapchain::destroy(VkDevice device) {
	for (auto& semaphore : renderSemaphores) {
		vkDestroySemaphore(device, semaphore, nullptr);
	}
	renderSemaphores.clear();

	for (auto& v : views) {
		vkDestroyImageView(device, v, nullptr);
	}
	views.clear();

	if (swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

	// REMINDER:
	// IMPLEMENT HDR
	// HDR MUST ONLY BE ENABLED IF:
	// 1) GPU supports HDR10 swapchain formats
	// 2) VK_EXT_swapchain_colorspace is available
	// 3) Monitor reports HDR10 support
	// 4) OS HDR mode is ON
	// For now just output as 8 bit as every monitor supports it

	for (auto& f : formats) {
		if (f.format == VK_FORMAT_B8G8R8A8_UNORM &&
			f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return f;
	}
	return formats[0];
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {

	for (auto& m : modes) {
		//older drivers and mobile phones may not support this
		if (m == VK_PRESENT_MODE_MAILBOX_KHR)
			return m;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& caps, uint32_t width, uint32_t height) {
	
	if (caps.currentExtent.width != UINT32_MAX) {
		return caps.currentExtent;
	}

	VkExtent2D e{};
	e.width = std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width);
	e.height = std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height);

	return e;
}

#pragma once
#include "volk/volk.h"
#include <vector>

class VulkanSwapchain {

public: 
	bool create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void destroy(VkDevice device);

	inline VkSwapchainKHR getSwapchain() const { return swapchain; }
	inline VkFormat getFormat() const { return imageFormat; }


private:

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps, uint32_t width, uint32_t height);

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D extent = {};

	std::vector<VkImage> images;
	std::vector<VkImageView> views;
};

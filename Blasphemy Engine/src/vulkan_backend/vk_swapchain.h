#pragma once
#include "volk/volk.h"
#include <vector>

class VulkanSwapchain {

public: 
	bool create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void destroy(VkDevice device);

	inline VkSwapchainKHR getSwapchain() const { return swapchain; }
	inline VkFormat getSwapchainImageFormat() const { return imageFormat; }
	VkExtent2D getExtent() const { return extent; }
	VkImage getSwapchainImage(std::uint32_t index) const { return images[index]; }

	std::uint32_t getSwapchainImageCount() const { return static_cast<std::uint32_t>(images.size()); }

	VkSemaphore getRenderSemaphore(uint32_t imageIndex) const { return renderSemaphores[imageIndex]; }


private:

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps, uint32_t width, uint32_t height);

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D extent{};

	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	std::vector<VkSemaphore> renderSemaphores;
};

#pragma once

#include "volk/volk.h"
#include <vma/vk_mem_alloc.h>

struct AllocatedImage {
	VkImage image{ VK_NULL_HANDLE };
	VkImageView imageView{ VK_NULL_HANDLE };
	VmaAllocation allocation{ VK_NULL_HANDLE };
	VkExtent3D imageExtent{ 0,0,1 };
	VkFormat imageFormat{ VK_FORMAT_UNDEFINED };
};

struct AllocatedBuffer {
	VkBuffer buffer{ VK_NULL_HANDLE };
	VmaAllocation allocation{ VK_NULL_HANDLE };
	VmaAllocationInfo info{};
	VkDeviceSize sizeBytes{ 0 };
};


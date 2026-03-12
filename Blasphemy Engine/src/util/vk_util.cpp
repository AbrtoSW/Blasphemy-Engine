#include "util/vk_util.h"

void DeletionQueue::flushFrameResources(VmaAllocator& vmaAllocator) {
	for (auto& b : vmaFrameAllocatedBuffer) {
		if (b.buffer != VK_NULL_HANDLE && b.allocation != VK_NULL_HANDLE) {
			vmaDestroyBuffer(vmaAllocator, b.buffer, b.allocation);
			b.buffer = VK_NULL_HANDLE;
			b.allocation = VK_NULL_HANDLE;
		}
	}
	vmaFrameAllocatedBuffer.clear(); // clear after destruction to prevent double frees
}

void DeletionQueue::flushMainResources(VkDevice device, VmaAllocator& vmaAllocator) {
	for (auto& aImg : allocatedImages) {
		vkDestroyImageView(device, aImg.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, aImg.image, aImg.allocation);
	}
	allocatedImages.clear();

	for (auto& offs : offscreenImages) {
		vkDestroyImageView(device, offs.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, offs.image, offs.allocation);
	}
	offscreenImages.clear();

	for (auto& b : vmaAllocatedBuffer) {
		vmaDestroyBuffer(vmaAllocator, b.buffer, b.allocation);
	}
	vmaAllocatedBuffer.clear();

}

void DeletionQueue::flushResize(VkDevice device, VmaAllocator& vmaAllocator) {
	for (auto& offs : offscreenImages) {
		if (offs.imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, offs.imageView, nullptr);
			offs.imageView = VK_NULL_HANDLE;
		}
		if (offs.image != VK_NULL_HANDLE) {
			vmaDestroyImage(vmaAllocator, offs.image, offs.allocation);
			offs.image = VK_NULL_HANDLE;
		}
	}
	offscreenImages.clear();
}


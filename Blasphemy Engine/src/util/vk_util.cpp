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

	for (auto& fb : framebuffers) {
		if (fb != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device, fb, nullptr);
			fb = VK_NULL_HANDLE;
		}
	}
	framebuffers.clear();

	for (auto& rp : renderPasses) {
		if (rp != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, rp, nullptr);
			rp = VK_NULL_HANDLE;
		}
	}
	renderPasses.clear();

	for (auto& aImg : allocatedImages) {
		if (aImg.imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, aImg.imageView, nullptr);
			aImg.imageView = VK_NULL_HANDLE;
		}
		if (aImg.image != VK_NULL_HANDLE && aImg.allocation != VK_NULL_HANDLE) {
			vmaDestroyImage(vmaAllocator, aImg.image, aImg.allocation);
			aImg.image = VK_NULL_HANDLE;
			aImg.allocation = VK_NULL_HANDLE;
		}
	}
	allocatedImages.clear();

	for (auto& offs : offscreenImages) {
		if (offs.imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, offs.imageView, nullptr);
			offs.imageView = VK_NULL_HANDLE;
		}
		if (offs.image != VK_NULL_HANDLE && offs.allocation != VK_NULL_HANDLE) {
			vmaDestroyImage(vmaAllocator, offs.image, offs.allocation);
			offs.image = VK_NULL_HANDLE;
			offs.allocation = VK_NULL_HANDLE;
		}
	}
	offscreenImages.clear();

	for (auto& b : vmaAllocatedBuffer) {
		if (b.buffer != VK_NULL_HANDLE && b.allocation != VK_NULL_HANDLE) {
			vmaDestroyBuffer(vmaAllocator, b.buffer, b.allocation);
			b.buffer = VK_NULL_HANDLE;
			b.allocation = VK_NULL_HANDLE;
		}
	}
	vmaAllocatedBuffer.clear();

}

void DeletionQueue::flushResize(VkDevice device, VmaAllocator& vmaAllocator) {

	for (auto& fb : framebuffers) {
		if (fb != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device, fb, nullptr);
			fb = VK_NULL_HANDLE;
		}
	}
	framebuffers.clear();

	for (auto& rp : renderPasses) {
		if (rp != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, rp, nullptr);
			rp = VK_NULL_HANDLE;
		}
	}
	renderPasses.clear();

	for (auto& offs : offscreenImages) {
		if (offs.imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, offs.imageView, nullptr);
			offs.imageView = VK_NULL_HANDLE;
		}
		if (offs.image != VK_NULL_HANDLE && offs.allocation != VK_NULL_HANDLE) {
			vmaDestroyImage(vmaAllocator, offs.image, offs.allocation);
			offs.image = VK_NULL_HANDLE;
			offs.allocation = VK_NULL_HANDLE;
		}
	}
	offscreenImages.clear();
}


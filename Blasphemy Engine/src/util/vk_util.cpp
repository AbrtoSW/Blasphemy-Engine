#include "util/vk_util.h"

void DeletionQueue::push_deletion_lambda(std::function<void()>&& function) {
	deletors.push_back(function);
}

void DeletionQueue::flush_deletion_lambda() {
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)();
	}
	deletors.clear();
}

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


	for (auto& s : samplers) {
		vkDestroySampler(device, s, nullptr);
	}
	for (auto& aImg : allocatedImages) {

		vkDestroyImageView(device, aImg.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, aImg.image, aImg.allocation);
	}

	for (auto& offs : offscreenImages) {
		vkDestroyImageView(device, offs.imageView, nullptr);
		vmaDestroyImage(vmaAllocator, offs.image, offs.allocation);
	}

	for (auto& b : vmaAllocatedBuffer) {
		vmaDestroyBuffer(vmaAllocator, b.buffer, b.allocation);
	}

	//ImGui_ImplVulkan_Shutdown();

	for (auto& d : descriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(device, d, nullptr);
	}

	for (auto& p : descriptorPools) {
		vkDestroyDescriptorPool(device, p, nullptr);
	}


	for (auto& p : pipelines) {
		vkDestroyPipeline(device, p, nullptr);
	}

	for (auto& l : pipelineLayouts) {
		vkDestroyPipelineLayout(device, l, nullptr);
	}


	for (auto& f : framebuffer) {
		vkDestroyFramebuffer(device, f, nullptr);
	}

	for (auto& r : renderpass) {
		vkDestroyRenderPass(device, r, nullptr);
	}

	for (auto& f : fences) {
		vkDestroyFence(device, f, nullptr);
	}

	for (auto& c : commandPool) {
		vkDestroyCommandPool(device, c, nullptr);
	}

	vmaDestroyAllocator(vmaAllocator);

}

void DeletionQueue::resizeFlush(VkDevice device, VmaAllocator& vmaAllocator) {

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
	for (auto& fb : framebuffer) {
		if (fb != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device, fb, nullptr);
			fb = VK_NULL_HANDLE;
		}
	}
}

#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"
#include "vulkan_backend/vk_frame_manager.h"
#include <array>
#include <iostream>



void Renderer::createSwapchainFramebuffer() {

	swapchainFrameBuffers.resize(vkBackend.getSwapchainImageCount());
	
	for (std::uint32_t i = 0; i < vkBackend.getSwapchainImageCount(); ++i) {

		VkImageView attachments[]{vkBackend.getSwapchainImageView(i)}; 

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = swapchainRenderPass; 
		framebufferInfo.attachmentCount = 1; 
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vkBackend.getSwapchainExtent().width; 
		framebufferInfo.height = vkBackend.getSwapchainExtent().height;
		framebufferInfo.layers = 1; 

		VK_CHECK(vkCreateFramebuffer(vkBackend.getDevice(), &framebufferInfo, nullptr, &swapchainFrameBuffers[i]));
	}
}

void Renderer::createDrawImageFramebuffer() {

	for (std::uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		std::array<VkImageView, 2> attachments = { drawImages[i].imageView, depthImages[i].imageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = drawImageRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = drawImages[i].imageExtent.width;
		framebufferInfo.height = drawImages[i].imageExtent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(vkBackend.getDevice(), &framebufferInfo, nullptr, &drawImageFramebuffers[i]));
	}
}

void Renderer::createFramebuffers() {
	std::cout << "[Renderer] create framebuffers...\n" << std::flush;

	createSwapchainFramebuffer();
	createDrawImageFramebuffer();
}


void Renderer::enqueueFramebuffersForDeletion() {

	for (std::uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		rendererDeletionQueue.pushFramebuffer(drawImageFramebuffers[i]);
	}

	for (VkFramebuffer& fb : swapchainFrameBuffers) {
		rendererDeletionQueue.pushFramebuffer(fb);
	}

}


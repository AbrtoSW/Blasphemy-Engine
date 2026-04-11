#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"
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

	std::array<VkImageView, 2> attachments = { drawImage.imageView, depthImage.imageView }; 

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = drawImageRenderPass; 
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = drawImage.imageExtent.width; 
	framebufferInfo.height = drawImage.imageExtent.height;
	framebufferInfo.layers = 1; 

	VK_CHECK(vkCreateFramebuffer(vkBackend.getDevice(), &framebufferInfo, nullptr, &drawImageFrameBuffer));
}

void Renderer::createFramebuffers() {
	std::cout << "[Renderer] create framebuffers...\n" << std::flush;

	createSwapchainFramebuffer();
	createDrawImageFramebuffer();
}


void Renderer::enqueueFramebuffersForDeletion() {
	
	rendererDeletionQueue.pushFramebuffer(drawImageFrameBuffer);

	for (VkFramebuffer& fb : swapchainFrameBuffers) {
		rendererDeletionQueue.pushFramebuffer(fb);
	}

}


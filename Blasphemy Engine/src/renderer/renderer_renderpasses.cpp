#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"


#include <array>

void Renderer::createDrawImageRenderpass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = drawImage.imageFormat; // VK_FORMAT_R16G16B16A16_SFLOAT
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // we load because we get an image from background pipelines, do clear if we dont render from background anymore
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthImage.imageFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	//fix this 
	std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vkBackend.getDevice(), &renderPassInfo, nullptr, &drawImageRenderPass));

	//fix this
}

void Renderer::createSwapchainRenderpass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vkBackend.getSwapChainImageFormat(); // e.g., VK_FORMAT_B8G8R8A8_UNORM
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Preserve copied data
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for vkCmdCopyImage
	dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Copy writes
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vkBackend.getDevice(), &renderPassInfo, nullptr, &swapchainRenderPass));



}

void Renderer::initDrawImageRenderpass(VkCommandBuffer cmd) {

	if (vkBackend.getDevice() == VK_NULL_HANDLE) {
		throw std::runtime_error("Cannot initialize render pass: invalid device");
	}
	if (drawImage.imageFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("Cannot initialize render pass: invalid draw image format");
	}
	if (depthImage.imageFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("Cannot initialize render pass: invalid depth image format");
	}


	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // color attachment
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = drawImageRenderPass;
	renderPassBeginInfo.framebuffer = drawImageFrameBuffer;
	renderPassBeginInfo.renderArea.extent = { drawExtent.width, drawExtent.height };
	renderPassBeginInfo.renderArea.offset = { 0,0 };

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();


	//start rendering 
	vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//render_pass_geometry(cmd, item, context);
	
	vkCmdEndRenderPass(cmd);

}

void Renderer::initSwapchainRenderpass(VkCommandBuffer cmd, uint32_t imageIndex) {

	if (vkBackend.getDevice() == VK_NULL_HANDLE) {
		throw std::runtime_error("Cannot initialize render pass: invalid device");
	}

	if (vkBackend.getSwapChainImageFormat() == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("Cannot initialize render pass: invalid swapchain image format");
	}

	VkClearValue clearValue{};
	clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = swapchainRenderPass;
	renderPassBeginInfo.framebuffer = swapchainFrameBuffers[imageIndex];
	renderPassBeginInfo.renderArea.extent = { vkBackend.getSwapchainExtent().width, vkBackend.getSwapchainExtent().height };
	renderPassBeginInfo.renderArea.offset = { 0,0 };

	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;


	//start rendering 
	vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//render_imgui(cmd);

	vkCmdEndRenderPass(cmd);
	//end rendering 

}

void Renderer::createRenderpasses() {
	createDrawImageRenderpass();
	createSwapchainRenderpass();
}


void Renderer::enqueueRenderPassessForDeletion() {

	rendererDeletionQueue.pushRenderPass(drawImageRenderPass);
	rendererDeletionQueue.pushRenderPass(swapchainRenderPass);
}

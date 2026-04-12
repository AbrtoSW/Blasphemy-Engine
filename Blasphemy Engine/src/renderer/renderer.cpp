#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"
#include "util/vk_helper.h"
#include "util/vk_util.h"
#include <iostream>


bool Renderer::initRenderer() {

	createOffscreenTargets();
	createRenderpasses();
	createFramebuffers();
	createDescriptors();
	createPipelines();

	return true;
}

void Renderer::renderFrame() {

	FrameData& frame = vkBackend.getCurrentFrame();

	VK_CHECK(vkWaitForFences(vkBackend.getDevice(), 1, &frame.renderFence, VK_TRUE, UINT64_MAX));

	std::uint32_t swapchainImageIndex{ 0 };

	VkResult result = vkAcquireNextImageKHR(vkBackend.getDevice(), vkBackend.getSwapchainHandle(), UINT64_MAX, frame.swapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		vkBackend.resizeRequested = true;
		return;
	}

	VkSemaphore currentRenderSemaphore = vkBackend.getRenderSemaphore(swapchainImageIndex);

	drawExtent.width = std::min(vkBackend.getSwapchainExtent().width, drawImage.imageExtent.width);
	drawExtent.height = std::min(vkBackend.getSwapchainExtent().height, drawImage.imageExtent.height);

	VK_CHECK(vkResetFences(vkBackend.getDevice(), 1, &frame.renderFence));

	VkCommandBuffer cmd{ frame.mainCommandBuffer };

	VkCommandBufferBeginInfo cmdBeginInfo = vkhelper::command_buffer_begin_info();

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//render graph goes here will be implemented soon

	

	vkhelper::transition_image(cmd, drawImage.image, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

	vkhelper::transition_image(cmd, drawImage.image, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
	
	vkhelper::transition_image(cmd, vkBackend.getSwapChainImage(swapchainImageIndex), 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

	vkhelper::transition_image(cmd, vkBackend.getSwapChainImage(swapchainImageIndex), VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);


	VK_CHECK(vkEndCommandBuffer(cmd));


	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	
	VkSubmitInfo submit = vkhelper::submit_info(&cmd, &currentRenderSemaphore, &frame.swapchainSemaphore, &waitStage);

	VK_CHECK(vkQueueSubmit(vkBackend.getGraphicsQueue(), 1, &submit, frame.renderFence));

	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	VkSwapchainKHR swapchain = vkBackend.getSwapchainHandle();
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &currentRenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VkResult presentResult = vkQueuePresentKHR(vkBackend.getGraphicsQueue(), &presentInfo);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
		vkBackend.resizeRequested = true;
		return;
	}

	//increase the number of frames drawn
	vkBackend.advanceFrame();
}

void Renderer::createOffscreenTargets() {

	std::cout << "[Renderer] createOffscreenTargets begin\n" << std::flush;

	VkExtent3D drawImageExtent = {
	vkBackend.getSwapchainExtent().width,
	vkBackend.getSwapchainExtent().height,
	1
	};

	drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	drawImage.imageExtent = drawImageExtent;

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo rimg_info = vkhelper::image_create_info(drawImage.imageFormat, drawImageUsages, drawImageExtent);

	std::cout << "[Renderer] vmaCreateImage drawImage...\n" << std::flush;
	VK_CHECK(vmaCreateImage(vkBackend.getVmaAllocator(), &rimg_info, &rimg_allocinfo, &drawImage.image, &drawImage.allocation, nullptr));

	VkImageViewCreateInfo rview_info = vkhelper::imageview_create_info(drawImage.imageFormat, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	std::cout << "[Renderer] vkCreateImageView drawImage...\n" << std::flush;
	VK_CHECK(vkCreateImageView(vkBackend.getDevice(), &rview_info, nullptr, &drawImage.imageView));
	rendererDeletionQueue.pushOffscreenImage(drawImage);


	depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
	depthImage.imageExtent = drawImageExtent;
	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = vkhelper::image_create_info(depthImage.imageFormat, depthImageUsages, drawImageExtent);
	std::cout << "[Renderer] vmaCreateImage depthImage...\n" << std::flush;
	VK_CHECK(vmaCreateImage(vkBackend.getVmaAllocator(), &dimg_info, &rimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr));

	VkImageViewCreateInfo dview_info = vkhelper::imageview_create_info(depthImage.imageFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);


	std::cout << "[Renderer] vkCreateImageView depthImage...\n" << std::flush;
	VK_CHECK(vkCreateImageView(vkBackend.getDevice(), &dview_info, nullptr, &depthImage.imageView));
	
	//might change deletion here
	rendererDeletionQueue.pushOffscreenImage(depthImage);

	std::cout << "[Renderer] createOffscreenTargets end\n" << std::flush;
}

void Renderer::destroyOffscreenTargets() {
	rendererDeletionQueue.flushResize(vkBackend.getDevice(), vkBackend.getVmaAllocator());
	drawImage = {};
	depthImage = {};
	drawImageRenderPass = VK_NULL_HANDLE;
	swapchainRenderPass = VK_NULL_HANDLE;
	drawImageFrameBuffer = VK_NULL_HANDLE;
	swapchainFrameBuffers.clear();
}

void Renderer::cleanup() {

	enqueueRenderPassessForDeletion();
	enqueueFramebuffersForDeletion();
	destroyOffscreenTargets();
	rendererDeletionQueue.flushMainResources(vkBackend.getDevice(), vkBackend.getVmaAllocator());
}




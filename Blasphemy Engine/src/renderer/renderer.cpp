#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"
#include "util/vk_helper.h"
#include "util/vk_util.h"

void Renderer::initRenderer() {

	createRenderpasses();
	createFramebuffers();
	//initDescriptors();

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

	vkhelper::transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vkhelper::transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vkhelper::transition_image(cmd, vkBackend.getSwapChainImage(swapchainImageIndex), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);




	VK_CHECK(vkEndCommandBuffer(cmd));



	VkCommandBufferSubmitInfo cmdinfo = vkhelper::command_buffer_submit_info(cmd);

	VkSemaphoreSubmitInfo waitInfo = vkhelper::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame.swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = vkhelper::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, currentRenderSemaphore);

	VkSubmitInfo2 submit = vkhelper::submit_info(&cmdinfo, &signalInfo, &waitInfo);

	VK_CHECK(vkQueueSubmit2(engine.graphicsQueue, 1, &submit, frame.renderFence));

	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &vkBackend.getSwapchainHandle();
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
	engine.frameNumber++;


}

void Renderer::createOffscreenTargets() {

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

	vmaCreateImage(vkBackend.getVmaAllocator(), &rimg_info, &rimg_allocinfo, &drawImage.image, &drawImage.allocation, nullptr);

	VkImageViewCreateInfo rview_info = vkhelper::imageview_create_info(drawImage.imageFormat, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(vkBackend.getDevice(), &rview_info, nullptr, &drawImage.imageView));
	rendererDeletionQueue.pushOffscreenImage(drawImage);


	depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
	depthImage.imageExtent = drawImageExtent;
	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = vkhelper::image_create_info(depthImage.imageFormat, depthImageUsages, drawImageExtent);
	vmaCreateImage(vkBackend.getVmaAllocator(), &dimg_info, &rimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr);

	VkImageViewCreateInfo dview_info = vkhelper::imageview_create_info(depthImage.imageFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);


	VK_CHECK(vkCreateImageView(vkBackend.getDevice(), &dview_info, nullptr, &depthImage.imageView));
	rendererDeletionQueue.pushOffscreenImage(depthImage);

	createDrawImageRenderpass();
	createSwapchainRenderpass();
	createDrawImageFramebuffer();
	createSwapchainFramebuffer();

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




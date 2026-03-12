#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"

void Renderer::renderFrame() {

	FrameData& frame = vulkanBackend.getCurrentFrame();

	VK_CHECK(vkWaitForFences(vulkanBackend.getDevice(), 1, &frame.renderFence, VK_TRUE, UINT64_MAX));

	std::uint32_t swapchainImageIndex{ 0 };

	VkResult result = vkAcquireNextImageKHR(vulkanBackend.getDevice(), vulkanBackend.getSwapchainHandle(), UINT64_MAX, frame.swapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);



}
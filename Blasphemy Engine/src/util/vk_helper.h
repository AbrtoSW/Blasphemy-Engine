#pragma once
#include "volk/volk.h"

namespace vkhelper {
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1);

	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry = "main");

	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);
	VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	// Classic (non-sync2) submit helpers.
	// NOTE: returned VkSubmitInfo stores pointers; caller must keep the pointed-to values alive until submission.
	VkSubmitInfo submit_info(VkCommandBuffer* cmd, VkSemaphore* signalSemaphore, VkSemaphore* waitSemaphore, VkPipelineStageFlags* waitDstStageMask);
	
	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);

	VkPipelineLayoutCreateInfo pipeline_layout_create_info();

	//void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags srcAccess, VkImageAspectFlags dstAccess, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range, VkImageAspectFlags srcStage, VkImageAspectFlags dstStage);
	void transition_image(VkCommandBuffer cmd, VkImage image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImageAspectFlags aspect, uint32_t baseMip, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount);
	void transition_image(VkCommandBuffer cmd, VkImage image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, const VkImageSubresourceRange& subresourceRange);
}
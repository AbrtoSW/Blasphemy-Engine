#pragma once
#include "types/vk_types.h"
#include "util/vk_util.h"
#include "descriptors/descriptors.h"
#include "pipelines/pipeline_types.h"

#include <vector>

class VulkanBackend;
class PipelineManager;

class Renderer {

public:

	Renderer(VulkanBackend& vulkanBackend, PipelineManager& pipelineManager) : vkBackend(vulkanBackend), pipelineManager(pipelineManager) {};

	bool initRenderer();
	
	void renderFrame();

	void createOffscreenTargets();

	void destroyOffscreenTargets();

	void cleanup();

private:

	VulkanBackend& vkBackend;
	PipelineManager& pipelineManager;

	VkExtent2D drawExtent{};

	AllocatedImage drawImage{};
	AllocatedImage depthImage{};

	VkRenderPass drawImageRenderPass{};
	VkRenderPass swapchainRenderPass{};

	std::vector<VkFramebuffer> swapchainFrameBuffers{};
	VkFramebuffer drawImageFrameBuffer{};

	DeletionQueue rendererDeletionQueue{};

	DescriptorAllocatorGrowable mainDescriptorAllocator{};

	VkDescriptorSet drawImageDescriptorSet{};
	VkDescriptorSetLayout drawImageDescriptorSetLayout{};

	PipelineID drawImagePipelineID{};
	
	
	void createRenderpasses();
	void createFramebuffers();


	void enqueueRenderPassessForDeletion();
	void enqueueFramebuffersForDeletion();
	void createDrawImageRenderpass();
	void createSwapchainRenderpass();
	
	void createSwapchainFramebuffer();
	void createDrawImageFramebuffer();

	void initDrawImageRenderpass(VkCommandBuffer cmd);
	void initSwapchainRenderpass(VkCommandBuffer cmd, uint32_t imageIndex);
	void createDescriptors();
	void initDrawImageDescriptor();
	void initFrameDescriptors();
	void createPipelines();
	void drawImagePipeline();
};
#pragma once
#include "types/vk_types.h"
#include "util/vk_util.h"
#include "descriptors/descriptors.h"
#include "pipelines/pipeline_types.h"
#include "vulkan_backend/vk_frame_manager.h"

#include <array>
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

	void createDescriptors();
	void createFramebuffers();

private:

	VulkanBackend& vkBackend;
	PipelineManager& pipelineManager;

	VkExtent2D drawExtent{};

	std::array<AllocatedImage, FRAME_OVERLAP> drawImages{};
	std::array<AllocatedImage, FRAME_OVERLAP> depthImages{};

	VkRenderPass drawImageRenderPass{};
	VkRenderPass swapchainRenderPass{};

	std::vector<VkFramebuffer> swapchainFrameBuffers{};
	std::array<VkFramebuffer, FRAME_OVERLAP> drawImageFramebuffers{};

	DeletionQueue rendererDeletionQueue{};

	DescriptorAllocatorGrowable mainDescriptorAllocator{};

	VkSampler drawImageSampler{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, FRAME_OVERLAP> drawImageDescriptorSets{};
	VkDescriptorSetLayout drawImageDescriptorSetLayout{};

	PipelineID drawImagePipelineID{};
	
	
	void createRenderpasses();
	void recordSceneGeometry(VkCommandBuffer cmd);
	void recordPostProcessPass(VkCommandBuffer cmd, uint32_t frameIndex);


	void enqueueRenderPassessForDeletion();
	void enqueueFramebuffersForDeletion();
	void createDrawImageRenderpass();
	void createSwapchainRenderpass();
	
	void createSwapchainFramebuffer();
	void createDrawImageFramebuffer();

	void recordDrawImagePass(VkCommandBuffer cmd, uint32_t frameIndex);
	void recordSwapchainPass(VkCommandBuffer cmd, uint32_t imageIndex, uint32_t frameIndex);
	void initDrawImageDescriptor();
	void initFrameDescriptors();
	void createPipelines();
	void drawImagePipeline();
};
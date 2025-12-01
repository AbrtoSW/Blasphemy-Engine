#pragma once
#include <volk/volk.h>
#include <vector>

enum class PipelineType {
	Uninitialized,
	Graphics,
	Compute,
	RayTracing
};

struct BaseGraphicsPipelineConfig {
	VkPipelineInputAssemblyStateCreateInfo     inputAssembly{};
	VkPipelineRasterizationStateCreateInfo     rasterizer{};
	VkPipelineMultisampleStateCreateInfo       multisampling{};
	VkPipelineDepthStencilStateCreateInfo      depthStencil{};
	VkPipelineColorBlendAttachmentState        colorBlendAttachment{};
	VkFormat                                   colorAttachmentFormat{};

	VkPipelineDynamicStateCreateInfo           dynamicStateInfo{};
	std::vector<VkDynamicState>                dynamicStates;

	VkPipelineViewportStateCreateInfo          viewportStateInfo{};
	VkPipelineVertexInputStateCreateInfo       vertexInputInfo{};
	VkPipelineColorBlendStateCreateInfo        colorBlendingInfo{};

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkRenderPass                               renderPass = VK_NULL_HANDLE;
	uint32_t                                   subpass = 0;

	std::vector<VkDescriptorSetLayout>         setLayouts;

	VkPushConstantRange                        pushConstantRange{};
};

struct PipelineRes {
	VkPipeline          pipeline = VK_NULL_HANDLE;
	VkPipelineLayout    pLayout = VK_NULL_HANDLE;

	BaseGraphicsPipelineConfig config;
	PipelineType type = PipelineType::Uninitialized;
};

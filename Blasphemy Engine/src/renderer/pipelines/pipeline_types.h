#pragma once
#include "volk/volk.h"
#include "renderer/shader/shader_types.h"
#include <vector>

using PipelineID = uint32_t;

enum class Hotloadable {
	False,
	True
};

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
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

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
	
	VkPipeline          pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout    pLayout{ VK_NULL_HANDLE };

	Shader shader;
	BaseGraphicsPipelineConfig config;
	PipelineID id{ 0 };
	PipelineType type{ PipelineType::Uninitialized };
	const char* name;
};

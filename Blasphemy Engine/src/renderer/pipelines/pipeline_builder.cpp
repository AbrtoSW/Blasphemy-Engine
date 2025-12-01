#include "pipeline_builder.h"
#include <iostream>

void PipelineBuilder::clear() {
	cfg = {};

	auto& c = cfg.config;

	c.inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	c.rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	c.multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	c.depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	c.colorBlendAttachment = {};

	c.dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	c.viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	c.vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	c.colorBlendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };

	c.shaderStages.clear();
	c.dynamicStates.clear();
	c.setLayouts.clear();
}

void PipelineBuilder::setPipelineLayout(VkPipelineLayout layout) {
	cfg.pLayout = layout;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, PipelineRes* out) {
	auto& c = cfg.config;

	c.viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	c.viewportStateInfo.viewportCount = 1;
	c.viewportStateInfo.scissorCount = 1;

	c.colorBlendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	c.colorBlendingInfo.logicOpEnable = VK_FALSE;
	c.colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
	c.colorBlendingInfo.attachmentCount = 1;

	c.vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	c.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	c.dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	c.dynamicStateInfo.pDynamicStates = c.dynamicStates.data();
	c.dynamicStateInfo.dynamicStateCount = (uint32_t)c.dynamicStates.size();

	VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	info.stageCount = (uint32_t)c.shaderStages.size();
	info.pStages = c.shaderStages.data();
	info.pVertexInputState = &c.vertexInputInfo;
	info.pInputAssemblyState = &c.inputAssembly;
	info.pViewportState = &c.viewportStateInfo;
	info.pRasterizationState = &c.rasterizer;
	info.pMultisampleState = &c.multisampling;
	info.pColorBlendState = &c.colorBlendingInfo;
	info.pDepthStencilState = &c.depthStencil;
	info.pDynamicState = &c.dynamicStateInfo;

	info.layout = cfg.pLayout;
	info.renderPass = c.renderPass;
	info.subpass = c.subpass;

	VkPipeline pipeline = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, nullptr, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	cfg.pipeline = pipeline;

	if (out) {
		out->pipeline = pipeline;
		out->pLayout = cfg.pLayout;
		out->config = cfg.config;
	}

	return pipeline;
}

void PipelineBuilder::set_shaders(VkShaderModule vert, VkShaderModule frag) {
	auto& c = cfg.config;

	c.shaderStages.clear();

	VkPipelineShaderStageCreateInfo vs{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vs.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vs.module = vert;
	vs.pName = "main";

	VkPipelineShaderStageCreateInfo fs{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fs.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fs.module = frag;
	fs.pName = "main";

	c.shaderStages.push_back(vs);
	c.shaderStages.push_back(fs);
}

void PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
	cfg.config.rasterizer.polygonMode = mode;
	cfg.config.rasterizer.lineWidth = 1.0f;
}

void PipelineBuilder::set_cull_mode(VkCullModeFlags cull, VkFrontFace frontFace) {
	cfg.config.rasterizer.cullMode = cull;
	cfg.config.rasterizer.frontFace = frontFace;
}

void PipelineBuilder::set_multisampling_none() {
	cfg.config.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	cfg.config.multisampling.sampleShadingEnable = VK_FALSE;
}

void PipelineBuilder::disable_blending() {
	auto& a = cfg.config.colorBlendAttachment;
	a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	a.blendEnable = VK_FALSE;
}

void PipelineBuilder::disable_depthtest() {
	auto& d = cfg.config.depthStencil;
	d.depthTestEnable = VK_FALSE;
	d.depthWriteEnable = VK_FALSE;
}

void PipelineBuilder::enable_depthtest(bool writeEnable, VkCompareOp op) {
	auto& d = cfg.config.depthStencil;
	d.depthTestEnable = VK_TRUE;
	d.depthWriteEnable = writeEnable;
	d.depthCompareOp = op;
}

void PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
	cfg.config.inputAssembly.topology = topology;
	cfg.config.inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::enable_blending_additive() {
	auto& a = cfg.config.colorBlendAttachment;
	a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	a.blendEnable = VK_TRUE;
	a.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	a.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	a.colorBlendOp = VK_BLEND_OP_ADD;
	a.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	a.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	a.alphaBlendOp = VK_BLEND_OP_ADD;
}

void PipelineBuilder::enable_blending_alphablend() {
	auto& a = cfg.config.colorBlendAttachment;
	a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	a.blendEnable = VK_TRUE;
	a.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	a.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	a.colorBlendOp = VK_BLEND_OP_ADD;
	a.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	a.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	a.alphaBlendOp = VK_BLEND_OP_ADD;
}

void PipelineBuilder::set_renderpass(VkRenderPass rp, uint32_t subpass) {
	cfg.config.renderPass = rp;
	cfg.config.subpass = subpass;
}

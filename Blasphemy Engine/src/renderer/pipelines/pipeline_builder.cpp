#include "pipeline_builder.h"
#include "util/vk_helper.h"

#include <iostream>

void PipelineBuilder::clear() {
	cfg = {};

	auto& c = cfg.config;

	c.inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	c.rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	c.multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	c.depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

	c.dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	c.viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	c.vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	c.colorBlendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };

	c.shaderStages.clear();
	c.dynamicStates.clear();
	c.setLayouts.clear();
	c.colorBlendAttachments.clear();

}

void PipelineBuilder::setPipelineLayout(VkPipelineLayout layout) {
	cfg.pLayout = layout;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, PipelineRes* out) {
	auto& c = cfg.config;

	c.viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	c.viewportStateInfo.viewportCount = 1;
	c.viewportStateInfo.scissorCount = 1;

	if (c.colorBlendAttachments.empty()) {
		setColorAttachmentCount(1);
	}

	c.colorBlendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	c.colorBlendingInfo.logicOpEnable = VK_FALSE;
	c.colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
	c.colorBlendingInfo.attachmentCount = (uint32_t)c.colorBlendAttachments.size();
	c.colorBlendingInfo.pAttachments = c.colorBlendAttachments.data();

	c.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
		if (out->type == PipelineType::Uninitialized) {
			std::cout << ".type is not initialized, hot loading shaders won't work\n";
		}

		out->pipeline = pipeline;
		out->pLayout = cfg.pLayout;
		out->config = cfg.config;
	}

	return pipeline;
}

void PipelineBuilder::set_shaders(VkShaderModule vert, VkShaderModule frag) {
	auto& c = cfg.config;

	c.shaderStages.clear();

	c.shaderStages.push_back(vkhelper::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vert));
	c.shaderStages.push_back(vkhelper::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, frag));

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
	auto& attachments = cfg.config.colorBlendAttachments;
	
	if (attachments.empty()) {
		setColorAttachmentCount(1);
	}

	VkPipelineColorBlendAttachmentState& a = attachments[0];
	a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
	auto& attachments = cfg.config.colorBlendAttachments;

	if (attachments.empty()) {
		setColorAttachmentCount(1);
	}
	VkPipelineColorBlendAttachmentState& a = attachments[0];
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
	auto& attachments = cfg.config.colorBlendAttachments;
	if (attachments.empty()) {
		setColorAttachmentCount(1);
	}

	VkPipelineColorBlendAttachmentState& a = attachments[0];
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

void PipelineBuilder::setColorAttachmentCount(uint32_t count) {
	auto& attachments = cfg.config.colorBlendAttachments;
	attachments.resize(count);

	for (auto& a : attachments) {
		a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		a.blendEnable = VK_FALSE;
		a.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		a.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		a.colorBlendOp = VK_BLEND_OP_ADD;
		a.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		a.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		a.alphaBlendOp = VK_BLEND_OP_ADD;
	}
}

#pragma once
#include "pipeline_types.h"

class PipelineBuilder {

public:
	PipelineRes cfg;

	PipelineBuilder() { clear(); }

	void clear();
	void setPipelineLayout(VkPipelineLayout layout);

	VkPipeline build_pipeline(VkDevice device, PipelineRes* out = nullptr);

	void set_shaders(VkShaderModule vert, VkShaderModule frag);
	void set_polygon_mode(VkPolygonMode mode);
	void set_cull_mode(VkCullModeFlags cull, VkFrontFace frontFace);
	void set_multisampling_none();
	void disable_blending();
	void disable_depthtest();
	void enable_depthtest(bool writeEnable, VkCompareOp op);
	void set_input_topology(VkPrimitiveTopology topology);
	void enable_blending_additive();
	void enable_blending_alphablend();
	void set_renderpass(VkRenderPass rp, uint32_t subpass = 0);
	void setColorAttachmentCount(uint32_t count);
};

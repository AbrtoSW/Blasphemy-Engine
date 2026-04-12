#include "renderer/renderer.h"
#include "engine/BlasphemyEngine.h"
#include "pipelines/pipeline_manager.h"
#include "pipelines/pipeline_builder.h"
#include "engine/EnginePaths.h"
#include "util/vk_helper.h"


void Renderer::createPipelines() {
	drawImagePipeline();
}


void Renderer::drawImagePipeline() {

	pipelineManager.drawImagePR.type = PipelineType::Graphics;

	pipelineManager.drawImagePR.shader.addShader(pipelineManager.getEnginePath(), "drawImageToneMap.vert", VK_SHADER_STAGE_VERTEX_BIT);
	pipelineManager.drawImagePR.shader.addShader(pipelineManager.getEnginePath(), "drawImageToneMap.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	const EnginePaths& ep = pipelineManager.getEnginePath();

	const auto& vertexInfo = pipelineManager.drawImagePR.shader.findShader(VK_SHADER_STAGE_VERTEX_BIT);

	if (!vertexInfo) {
		std::cerr << "Draw Image Pipeline vertex is either missing, wrong path, or wrong / missing stage";
		return;
	}

	const std::filesystem::path vertexPath = ep.gameAssets / vertexInfo->relativePath;

	VkShaderModule vertexShader = ShaderUtility::compileToSPV(vkBackend.getDevice(), vertexPath.string(), EShLangVertex, vertexPath.parent_path());


	const auto& fragmentInfo = pipelineManager.drawImagePR.shader.findShader(VK_SHADER_STAGE_FRAGMENT_BIT);

	if (!fragmentInfo) {
		std::cerr << "Draw Image Pipeline Fragment is either missing, wrong path, or wrong / missing stage";
		return;
	}

	const std::filesystem::path fragmentPath = ep.gameAssets / fragmentInfo->relativePath;

	VkShaderModule fragmentShader = ShaderUtility::compileToSPV(vkBackend.getDevice(), fragmentPath.string(), EShLangFragment, fragmentPath.parent_path());

	auto& cfg = pipelineManager.drawImagePR.config;

	cfg.layoutInfo = vkhelper::pipeline_layout_create_info();
	cfg.layoutInfo.pPushConstantRanges = nullptr;
	cfg.layoutInfo.pushConstantRangeCount = 0;

	cfg.setLayouts = { drawImageDescriptorSetLayout };
	cfg.layoutInfo.pSetLayouts = cfg.setLayouts.data();
	cfg.layoutInfo.setLayoutCount = static_cast<std::uint32_t>(cfg.setLayouts.size());

	
	if (vkCreatePipelineLayout(vkBackend.getDevice(), &cfg.layoutInfo, nullptr, &pipelineManager.drawImagePR.pLayout) != VK_SUCCESS) {
		std::cerr << "Pipeline Layout creation failed";
		vkDestroyShaderModule(vkBackend.getDevice(), fragmentShader, nullptr);
		return;
	}

	PipelineBuilder b{};
	b.setPipelineLayout(pipelineManager.drawImagePR.pLayout);
	b.set_shaders(vertexShader, fragmentShader);
	b.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	b.set_polygon_mode(VK_POLYGON_MODE_FILL);
	// Swapchain pass is color-only (no depth attachment); depth test must be off for valid pipeline compatibility.
	// Fullscreen triangle: disable culling so winding cannot drop the entire pass.
	b.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	b.set_multisampling_none();
	b.disable_blending();
	b.disable_depthtest();
	b.set_renderpass(swapchainRenderPass);

	pipelineManager.drawImagePR.pipeline = b.build_pipeline(vkBackend.getDevice(), &pipelineManager.drawImagePR);

	pipelineManager.registerPipeline(pipelineManager.drawImagePR, drawImagePipelineID, Hotloadable::True, "Tone Map Pipeline");


	vkDestroyShaderModule(vkBackend.getDevice(), vertexShader, nullptr);
	vkDestroyShaderModule(vkBackend.getDevice(), fragmentShader, nullptr);

}
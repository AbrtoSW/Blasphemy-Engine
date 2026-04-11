#include "renderer/renderer.h"
#include "engine/BlasphemyEngine.h"
#include "pipelines/pipeline_manager.h"
#include "engine/EnginePaths.h"


void Renderer::createPipelines() {

}


void Renderer::drawImagePipeline() {

	pipelineManager.drawImagePR.type = PipelineType::Graphics;

	pipelineManager.drawImagePR.shader.addShader(pipelineManager.getEnginePath(), "drawImageToneMapFrag.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	
	auto fragmentInfo = pipelineManager.drawImagePR.shader.findShader(VK_SHADER_STAGE_FRAGMENT_BIT);

	const EnginePaths& ep = pipelineManager.getEnginePath();

	const std::filesystem::path resolved = ep.gameAssets / fragmentInfo->relativePath;
	
	VkShaderModule fragmentShader = ShaderUtility::compileToSPV(vkBackend.getDevice(), resolved.string(), EShLangFragment, resolved.parent_path());

}
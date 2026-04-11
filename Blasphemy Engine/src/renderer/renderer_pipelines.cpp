#include "renderer/renderer.h"
#include "pipelines/pipeline_manager.h"


void Renderer::createPipelines() {

}


void Renderer::drawImagePipeline() {

	pipelineManager.drawImagePR.type = PipelineType::Graphics;

	pipelineManager.drawImagePR.shader.addShader(pipelineManager.getEnginePath(), "drawImageToneMapFrag.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

}
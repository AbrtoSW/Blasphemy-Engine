#include "renderer/pipelines/pipeline_manager.h"
#include "renderer/shader/shader_util.h"
#include <set>
#include <iostream>

void PipelineManager::registerPipeline(PipelineRes& pRes, Hotloadable hotload, std::optional<const char*> name /*= std::nullopt*/) {

	PipelineID id = pRes.id;

	if (id == 0) {
		id = nextID();
		pRes.id = id;
	}
	else {
		
		auto it = pipelineStorage.find(id);

		if (it != pipelineStorage.end()) {

			auto& old = it->second;

			if (old.pipeline != VK_NULL_HANDLE && queuedPipelines.insert(old.pipeline).second)
				deletionQueue.pushPipeline(old.pipeline);

			if (old.pLayout != VK_NULL_HANDLE && queuedLayouts.insert(old.pLayout).second)
				deletionQueue.pushLayout(old.pLayout);
		}
	}

	pipelineStorage[id] = pRes;

	if (name)
		pipelineStorage[id].name = *name;


	if (hotload == Hotloadable::True)
		track_shaders_for_hotload(pipelineStorage[id]);
}

void PipelineManager::track_shaders_for_hotload(PipelineRes& resource) {
	
	auto add = [&](const std::string& path) {
		if (path.empty()) return;
		auto& vec = shaderMap[path];
		if (std::find(vec.begin(), vec.end(), &resource) == vec.end()) {
			vec.push_back(&resource);
		}
	};

	for (auto& s : resource.shader.stages) {
		if (!s.file.relativePath.empty()) {
			add(s.file.relativePath);
			std::cout << "Registered stage: " << s.stage << "\n path: " << s.file.relativePath << "\n last time modified: " << s.lastModified.time_since_epoch().count() << "\n";
		}
	}
}

void PipelineManager::hotloadShader() {

	auto& shaderMap = getShaderMap();
	
	std::set<PipelineRes*> pipelinesToRebuild;

	for (auto& [file, resources] : shaderMap) {

		const auto newStamp = ShaderUtility::getFileTimeStamp(file);

		for (auto* r : resources) {

			for (auto& s : r->shader.stages) {

				if (s.file.relativePath == file) {

					if (newStamp != s.lastModified)
						pipelinesToRebuild.insert(r);
				}
			}
		}
	}

	//for (auto* r : pipelinesToRebuild) {

	//	VkPipeline rebuilt = rebuild();
	//	
	//	if (rebuilt == VK_NULL_HANDLE)
	//	continue;
	//	
	//	//make sure this logic is correct because if we hot reload again we'd basically need this pipeline to be destroyed again but its not being tracked by the original pipeline it was in, the logic might be tracked inside rebuild function 
	//	deletionQueue.pushPipeline(rebuilt);

	//	for (auto& s : r->shader.stages) {
	//		s.lastModified = ShaderUtility::getFileTimeStamp(s.file.relativePath);
	//	}
	//}
} 

//VkPipeline PipelineManager::rebuild(VkDevice device, PipelineRes& res) {
//
//	std::cout << "rebuild Pipelines called\n";
//
//	VkPipeline oldPipeline = res.pipeline;
//
//	auto& cfg = res.config;
//
//	std::cout << "old pipeline object identification is : " << (void*)oldPipeline << "\n";
//	std::cout << "RenderPass handle on rebuild: " << (void*)cfg.renderPass << "\n";
//
//	cfg.shaderStages.clear();
//
//
//
//	std::vector<VkShaderModule> modules;
//
//	for (auto& s : res.shader.stages) {
//
//		if (s.file.relativePath.empty())
//			continue;
//
//		VkShaderModule mod = ShaderUtility::compileToSPV(device, s.file.relativePath,
//			(s.stage == VK_SHADER_STAGE_VERTEX_BIT ? EShLangVertex :
//				s.stage == VK_SHADER_STAGE_FRAGMENT_BIT ? EShLangFragment :
//				s.stage == VK_SHADER_STAGE_GEOMETRY_BIT ? EShLangGeometry :
//				EShLangCompute));
//
//		if (!mod)
//			continue;
//
//		modules.push_back(mod);
//
//		cfg->shaderStages.push_back(
//			vkinit::pipeline_shader_stage_create_info(s.stage, mod));
//	}
//
//	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
//
//	if (cfg->renderMode == RenderMode::Dynamic) {
//		pipelineInfo.renderPass = VK_NULL_HANDLE;
//		pipelineInfo.subpass = 0;
//		pipelineInfo.pNext = &cfg->renderInfo;
//		cfg->renderPass = VK_NULL_HANDLE;
//	}
//	else {
//		pipelineInfo.renderPass = cfg->renderPass;
//		pipelineInfo.subpass = 0;
//		pipelineInfo.pNext = nullptr;
//	}
//
//	pipelineInfo.stageCount = (uint32_t)cfg->shaderStages.size();
//	pipelineInfo.pStages = cfg->shaderStages.data();
//	pipelineInfo.pVertexInputState = &cfg->vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &cfg->inputAssembly;
//	pipelineInfo.pViewportState = &cfg->viewportStateInfo;
//	pipelineInfo.pRasterizationState = &cfg->rasterizer;
//	pipelineInfo.pMultisampleState = &cfg->multisampling;
//	pipelineInfo.pColorBlendState = &cfg->colorBlendingInfo;
//	pipelineInfo.pDepthStencilState = &cfg->depthStencil;
//	pipelineInfo.pDynamicState = &cfg->dynamicStateInfo;
//	pipelineInfo.layout = res.pLayout;
//
//	fmt::print("Pipeline pointers:\n");
//	fmt::print("  pStages: {}\n", (void*)pipelineInfo.pStages);
//	fmt::print("  pVertexInputState: {}\n", (void*)pipelineInfo.pVertexInputState);
//	fmt::print("  pInputAssemblyState: {}\n", (void*)pipelineInfo.pInputAssemblyState);
//	fmt::print("  pViewportState: {}\n", (void*)pipelineInfo.pViewportState);
//	fmt::print("  pRasterizationState: {}\n", (void*)pipelineInfo.pRasterizationState);
//	fmt::print("  pMultisampleState: {}\n", (void*)pipelineInfo.pMultisampleState);
//	fmt::print("  pColorBlendState: {}\n", (void*)pipelineInfo.pColorBlendState);
//	fmt::print("  pDepthStencilState: {}\n", (void*)pipelineInfo.pDepthStencilState);
//	fmt::print("  layout: {}\n", (void*)pipelineInfo.layout);
//	fmt::print("  pDynamicState: {}\n", (void*)pipelineInfo.pDynamicState);
//	fmt::print("  renderPass: {}\n", (void*)pipelineInfo.renderPass);
//
//	VkPipeline newPipeline = VK_NULL_HANDLE;
//	VkResult vr = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &newPipeline);
//
//	if (vertexModule)   vkDestroyShaderModule(device, vertexModule, nullptr);
//	if (fragmentModule) vkDestroyShaderModule(device, fragmentModule, nullptr);
//
//	if (vr != VK_SUCCESS) {
//		fmt::println("Failed to rebuild pipeline");
//		return VK_NULL_HANDLE;
//	}
//
//	res.pipeline = newPipeline;
//	return newPipeline;
//}
//



void PipelineManager::destroyPipelineResources(VkDevice device) {
	deletionQueue.flush(device);
}

void PipelineManager::showInfo() {
	for (const auto& pair : pipelineStorage) {
		PipelineID id = pair.first;
		const auto& info = pair.second;
		std::cout << "ID = " << id << "\n" << "  Pipeline = " << (const void*)info.pipeline << "\n" << "  Layout   = " << (const void*)info.pLayout << "\n" << "  name = " << (info.name ? info.name : "didn't assign name") << "\n";
	}
}

VkPipeline PipelineManager::getPipeline(PipelineRes& res) const {
	auto it = pipelineStorage.find(res.id);
	return (it != pipelineStorage.end()) ? it->second.pipeline : VK_NULL_HANDLE;
}

VkPipelineLayout PipelineManager::getLayout(PipelineRes& res) const {
	auto it = pipelineStorage.find(res.id);
	return (it != pipelineStorage.end()) ? it->second.pLayout : VK_NULL_HANDLE;
}



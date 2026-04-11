#include "pipelines/pipeline_manager.h"
#include "shader/shader_util.h"
#include "util/vk_helper.h"
#include "vulkan_backend/vk_backend.h"

#include <set>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace {

EShLanguage vk_stage_to_glslang(VkShaderStageFlagBits stage) {
	switch (stage) {
	case VK_SHADER_STAGE_VERTEX_BIT: return EShLangVertex;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return EShLangTessControl;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLangTessEvaluation;
	case VK_SHADER_STAGE_GEOMETRY_BIT: return EShLangGeometry;
	case VK_SHADER_STAGE_FRAGMENT_BIT: return EShLangFragment;
	case VK_SHADER_STAGE_COMPUTE_BIT: return EShLangCompute;
	default: return EShLangCompute;
	}
}

} // namespace

void PipelineManager::registerPipeline(PipelineRes& pRes, PipelineID& rendererHandlePipelineID,Hotloadable hotload, std::optional<const char*> name /*= std::nullopt*/) {

	
	PipelineID id = pRes.id;

	if (id == 0) {
		id = nextID();
		pRes.id = id;
		rendererHandlePipelineID = pRes.id;
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
	rendererHandlePipelineID = pRes.id;
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

	for (auto* r : pipelinesToRebuild) {

		VkPipeline rebuilt = rebuild(*r);

		if (rebuilt == VK_NULL_HANDLE)
			continue;

		for (auto& s : r->shader.stages) {
			s.lastModified = ShaderUtility::getFileTimeStamp(s.file.relativePath);
		}
	}

} 

VkPipeline PipelineManager::rebuild(PipelineRes& res) {

	std::cout << "rebuild Pipelines called\n";

	VkPipeline oldPipeline { res.pipeline };

	BaseGraphicsPipelineConfig& cfg { res.config };

	std::cout << "old pipeline object identification is : " << (void*)oldPipeline << "\n";
	std::cout << "RenderPass handle on rebuild: " << (void*)cfg.renderPass << "\n";

	cfg.shaderStages.clear();


	std::vector<VkShaderModule> modules{};

	for (auto& s : res.shader.stages) {

		if (s.file.relativePath.empty())
			continue;

		VkShaderModule mod = ShaderUtility::compileToSPV(vkBackend.getDevice(), s.file.relativePath,
			vk_stage_to_glslang(s.stage), std::filesystem::path(s.file.relativePath).parent_path());

		if (!mod)
			continue;

		modules.push_back(mod);

		cfg.shaderStages.push_back(vkhelper::pipeline_shader_stage_create_info(s.stage, mod));
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	
	pipelineInfo.renderPass = cfg.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = (uint32_t)cfg.shaderStages.size();
	pipelineInfo.pStages = cfg.shaderStages.data();
	pipelineInfo.pVertexInputState = &cfg.vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &cfg.inputAssembly;
	pipelineInfo.pViewportState = &cfg.viewportStateInfo;
	pipelineInfo.pRasterizationState = &cfg.rasterizer;
	pipelineInfo.pMultisampleState = &cfg.multisampling;
	pipelineInfo.pColorBlendState = &cfg.colorBlendingInfo;
	pipelineInfo.pDepthStencilState = &cfg.depthStencil;
	pipelineInfo.pDynamicState = &cfg.dynamicStateInfo;
	pipelineInfo.layout = res.pLayout;

	std::cout << "Pipeline pointers:\n";
	std::cout << "  pStages: " << (void*)pipelineInfo.pStages << "\n";
	std::cout << "  pVertexInputState: " << (void*)pipelineInfo.pVertexInputState << "\n";
	std::cout << "  pInputAssemblyState: " << (void*)pipelineInfo.pInputAssemblyState << "\n";
	std::cout << "  pViewportState: " << (void*)pipelineInfo.pViewportState << "\n";
	std::cout << "  pRasterizationState: " << (void*)pipelineInfo.pRasterizationState << "\n";
	std::cout << "  pMultisampleState: " << (void*)pipelineInfo.pMultisampleState << "\n";
	std::cout << "  pColorBlendState: " << (void*)pipelineInfo.pColorBlendState << "\n";
	std::cout << "  pDepthStencilState: " << (void*)pipelineInfo.pDepthStencilState << "\n";
	std::cout << "  layout: " << (void*)pipelineInfo.layout << "\n";
	std::cout << "  pDynamicState: " << (void*)pipelineInfo.pDynamicState << "\n";
	std::cout << "  renderPass: " << (void*)pipelineInfo.renderPass << "\n";

	if (cfg.shaderStages.empty()) {
		std::cout << "Failed to rebuild pipeline: no shader stages compiled\n";
		return VK_NULL_HANDLE;
	}

	VkDevice device = vkBackend.getDevice();

	auto destroyModules = [&]() {
		for (VkShaderModule m : modules) {
			if (m != VK_NULL_HANDLE)
				vkDestroyShaderModule(device, m, nullptr);
		}
		modules.clear();
	};

	VkPipeline newPipeline = VK_NULL_HANDLE;
	VkResult vr = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &newPipeline);

	if (vr != VK_SUCCESS) {
		std::cout << "Failed to rebuild pipeline\n";
		destroyModules();
		return VK_NULL_HANDLE;
	}

	if (oldPipeline != VK_NULL_HANDLE && queuedPipelines.insert(oldPipeline).second)
		deletionQueue.pushPipeline(oldPipeline);

	res.pipeline = newPipeline;
	destroyModules();
	return newPipeline;
}




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

VkPipeline PipelineManager::getPipeline(PipelineID& id) const {
	auto it = pipelineStorage.find(id);
	return (it != pipelineStorage.end()) ? it->second.pipeline : VK_NULL_HANDLE;
}

VkPipelineLayout PipelineManager::getLayout(PipelineID& id) const {
	auto it = pipelineStorage.find(id);
	return (it != pipelineStorage.end()) ? it->second.pLayout : VK_NULL_HANDLE;
}



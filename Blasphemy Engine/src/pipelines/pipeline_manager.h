#pragma once

#include "pipelines/pipeline_types.h"
#include "shader/shader_types.h"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <optional>

class VulkanBackend;
class EnginePaths;

struct PipelineDeletionQueue {
	std::vector<VkPipeline>        pipelines;
	std::vector<VkPipelineLayout>  layouts;

	inline void pushPipeline(VkPipeline p) { pipelines.push_back(p); }
	inline void pushLayout(VkPipelineLayout l) { layouts.push_back(l); }

	inline void flush(VkDevice device) {
		for (auto p : pipelines) {
			vkDestroyPipeline(device, p, nullptr);
		}
		pipelines.clear();

		for (auto l : layouts) {
			vkDestroyPipelineLayout(device, l, nullptr);
		}
		layouts.clear();
	}
};

class PipelineManager {
public:

	PipelineManager(VulkanBackend& vkBackend, EnginePaths& ep) : vkBackend(vkBackend), enginePaths(ep){}
	
	inline static PipelineID nextID() {
		return pID++;
	}

	void registerPipeline(PipelineRes& pRes, PipelineID& rendererHandlePipelineID, Hotloadable hotload, std::optional<const char*> name = std::nullopt);
	void destroyPipelineResources(VkDevice device);
	void showInfo();
	VkPipeline getPipeline(PipelineID& id) const;
	VkPipelineLayout getLayout(PipelineID& id) const;
	void hotloadShader();
	VkPipeline rebuild(PipelineRes& res);

	EnginePaths getEnginePath() const { return enginePaths; }
	auto& getShaderMap() { return shaderMap; }

	PipelineRes drawImagePR{};


private:

	
	VulkanBackend& vkBackend;
	EnginePaths& enginePaths;

	PipelineDeletionQueue deletionQueue;

	std::unordered_map<PipelineID, PipelineRes> pipelineStorage;
	std::unordered_set<VkPipeline>	queuedPipelines;
	std::unordered_set<VkPipelineLayout> queuedLayouts;
	std::unordered_map<std::string, std::vector<PipelineRes*>> shaderMap;

	inline static PipelineID pID = 0;

	void track_shaders_for_hotload(PipelineRes& resource);

	

};

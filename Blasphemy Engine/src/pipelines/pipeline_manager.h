#pragma once

#include "pipelines/pipeline_types.h"
#include "shader/shader_types.h"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <optional>

class VulkanBackend;

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

	PipelineManager(VulkanBackend& vkBackend) : vkBackend(vkBackend){}
	
	inline static PipelineID nextID() {
		return pID++;
	}

	void registerPipeline(PipelineRes& pRes, Hotloadable hotload, std::optional<const char*> name = std::nullopt);
	void destroyPipelineResources(VkDevice device);
	void showInfo();
	VkPipeline getPipeline(PipelineRes& res) const;
	VkPipelineLayout getLayout(PipelineRes& res) const;
	void hotloadShader();
	VkPipeline rebuild(PipelineRes& res);


	auto& getShaderMap() { return shaderMap; }

private:

	VulkanBackend& vkBackend;
	PipelineDeletionQueue deletionQueue;

	std::unordered_map<PipelineID, PipelineRes> pipelineStorage;
	std::unordered_set<VkPipeline>	queuedPipelines;
	std::unordered_set<VkPipelineLayout> queuedLayouts;
	std::unordered_map<std::string, std::vector<PipelineRes*>> shaderMap;

	inline static PipelineID pID = 0;

	void track_shaders_for_hotload(PipelineRes& resource);

	

};

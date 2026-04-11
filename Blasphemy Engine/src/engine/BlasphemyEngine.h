#pragma once

#include <string>
#include "engine/EnginePaths.h"
#include "pipelines/pipeline_manager.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "vulkan_backend/vk_backend.h"

class BlasphemyEngine {

public:
	
	bool init();
	void run();
	void shutdown();

private:
	
	void typeOut(const std::string& s, int delay);

	void resizeWindow();

	std::uint64_t frameNumber{};
	Platform platform{};
	VulkanBackend vkBackend{ platform };
	EnginePaths paths{};
	PipelineManager pipelineManager{ vkBackend };
	Renderer renderer{ vkBackend , pipelineManager};

};



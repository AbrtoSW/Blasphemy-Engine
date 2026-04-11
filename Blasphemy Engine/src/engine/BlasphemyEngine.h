#pragma once

#include <string>
#include "platform/platform.h"
#include "vulkan_backend/vk_backend.h"
#include "renderer/renderer.h"


struct EnginePaths {
	std::filesystem::path content{};
};

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
	Renderer renderer{ vkBackend };

};



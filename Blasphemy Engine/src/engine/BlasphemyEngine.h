#pragma once

#include <string>
#include "platform/platform.h"
#include "vulkan_backend/vk_backend.h"
#include "renderer/renderer.h"

class BlasphemyEngine {

public:
	
	bool init();
	void run();
	void shutdown();

private:
	void typeOut(const std::string& s, int delay);

	std::uint64_t frameNumber{};
	Platform platform{};
	VulkanBackend vkBackend{ platform };
	Renderer renderer{ vkBackend, vkBackend.frameManager };

};



#pragma once

#include <string>
#include "platform/platform.h"
#include "vulkan_backend/vk_backend.h"

class BlasphemyEngine {

public:
	
	void init();
	void run();
	void shutdown();

private:
	void type_out(const std::string& s, int delay);

	Platform platform;
	VulkanBackend vkBackend{ platform };

};



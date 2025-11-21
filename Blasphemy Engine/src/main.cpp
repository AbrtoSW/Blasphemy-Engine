#include <iostream>
#include "platform/platform.h"
#include "engine/vk_engine.h"

int main() {

	Platform platform;
	if (!platform.init()) {
		std::cout << "Platform init failed.\n";
		return -1;
	}

	VulkanEngine engine(platform);
	engine.init();
	engine.run();
	//engine.cleanup();
	
	platform.shutdown();

	return 0;
}
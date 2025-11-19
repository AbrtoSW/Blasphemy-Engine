#pragma once

class Platform;

class VulkanEngine {

public:

	VulkanEngine(Platform& platform) 
		: platform(platform) {}

	void init();
	//void run();
	//void cleanup();

private:

	Platform& platform;

	bool isInitalized = { false };

};
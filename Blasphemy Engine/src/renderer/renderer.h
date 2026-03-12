#pragma once

class VulkanBackend;

class Renderer {

public:

	Renderer(VulkanBackend& vulkanBackend) : vulkanBackend(vulkanBackend){};

	void renderFrame();

private:

	VulkanBackend& vulkanBackend;
	
	
	
	void initDescriptors();

	




};
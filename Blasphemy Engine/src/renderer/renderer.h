#pragma once

class VulkanBackend;

class Renderer {

public:

	Renderer(VulkanBackend& backend) : backend(backend) {};

private:
	VulkanBackend& backend;

	void initDescriptors();

	void createRenderTargets();
	void createGBufferTargets();
	void createLightingTargets();
	void createTransparencyTargets();
	void createPostFXTargets();


};
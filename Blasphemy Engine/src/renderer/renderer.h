#pragma once

class VulkanBackend;
class FrameManager;

class Renderer {

public:

	Renderer(VulkanBackend& backend, FrameManager& frameManager) : backend(backend), frameManager(frameManager) {};

	void renderFrame();

private:

	VulkanBackend& backend;
	FrameManager& frameManager;
	
	
	
	void initDescriptors();

	void createRenderTargets();
	void createGBufferTargets();
	void createLightingTargets();
	void createTransparencyTargets();
	void createPostFXTargets();




};
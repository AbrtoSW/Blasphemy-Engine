#include "BlasphemyEngine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "platform/platform.h"
#include "vulkan_backend/vk_backend.h"



bool BlasphemyEngine::init() {

	typeOut("Initiating Blasphemy Engine" , 30);

	if (!platform.init()) {
		return false;
	}

	if (!vkBackend.init()) {
		return false;
	}

	return true;
}

void BlasphemyEngine::run() {

	std::cout << "Engine Entering main loop.\n";

	while (!platform.shouldClose()) {
		// 1) timing
		vkBackend.update_timing();

		// 2) poll platform events (SDL etc. lives in Platform)
		platform.pollEvents();

		// 3) per-frame engine work will go here later:
		//    - acquire swapchain image
		//    - get current FrameData
		//    - record command buffers
		//    - submit + present

		renderer.renderFrame();
	
		frameNumber++;
	}

	if (vkBackend.resizeRequested) {
		resizeWindow();
	}
	
	
	std::cout << "Engine Exiting main loop.\n";


}


void BlasphemyEngine::shutdown() {

	renderer.destroyOffscreenTargets();
	vkBackend.cleanup();
	platform.shutdown();

}


void BlasphemyEngine::typeOut(const std::string& s, int delay) {

	for (auto& c : s) {
		std::cout << c << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}
	std::cout << "\n";

}

void BlasphemyEngine::resizeWindow() {


	vkDeviceWaitIdle(vkBackend.getDevice());

	//renderer.resizeFlush / destroyFramebuffers check grotesk
	renderer.destroyOffscreenTargets();

	vkBackend.destroySwapchain();

	int width{}, height{};

	SDL_GetWindowSize(platform.getWindow(), &width, &height);

	if (width == 0 || height == 0) {
		return;
	}

	VkExtent2D windowExtent = platform.getWindowExtent();
	windowExtent = { (std::uint32_t)width, (std::uint32_t)height };
	vkBackend.recreateSwapchainResources();
	renderer.createOffscreenTargets();
	// renderer.initFramebuffers();
	// renderer.initDescriptors();

	vkBackend.resizeRequested = false;
	

}
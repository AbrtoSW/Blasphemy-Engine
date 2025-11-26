#include "BlasphemyEngine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "platform/platform.h"
#include "vulkan_backend/vk_backend.h"



void BlasphemyEngine::init() {

	type_out("Initiating Blasphemy Engine" , 30);

	if (!platform.init())
		return;

	vkBackend.init();


}

void BlasphemyEngine::run() {
	vkBackend.run();

}

void BlasphemyEngine::shutdown() {
	vkBackend.cleanup();
	platform.shutdown();
}


void BlasphemyEngine::type_out(const std::string& s, int delay) {
	for (auto& c : s) {
		std::cout << c << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}
	std::cout << "\n";
}
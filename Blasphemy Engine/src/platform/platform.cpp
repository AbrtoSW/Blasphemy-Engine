#include "platform/platform.h"
#include <SDL3/SDL_vulkan.h>

bool Platform::init() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return false;
	}

	mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());


	SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	window = SDL_CreateWindow("BLASPHEMY", (int)(windowExtent.width * mainScale), (int)(windowExtent.height * mainScale), flags);

	if (!window) {
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);


}

void Platform::shutdown() {
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_Quit();
}


void Platform::pollEvents() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			quit = true;
		}
		if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			windowExtent.width = e.window.data1;
			windowExtent.height = e.window.data2;
		}
	}
}

bool Platform::shouldClose() const {
	return quit;
}

SDL_Window* Platform::getWindow() const {
	return window;
}

VkExtent2D Platform::getWindowExtent() const {
	return windowExtent;
}

float Platform::getScale() const {
	return mainScale;
}

VkSurfaceKHR Platform::createVulkanSurface(VkInstance instance, VkAllocationCallbacks* allocator) {

	VkSurfaceKHR vkSurface = nullptr;
	
	if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &vkSurface)) {
		return nullptr;
	}
	return vkSurface;
}

void Platform::getRequiredVulkanExtensions(std::vector<const char*>& outExtensions) const {
	uint32_t count = 0;
	const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&count);
	for (uint32_t i = 0; i < count; ++i) {
		outExtensions.push_back(exts[i]);
	}
}

const std::filesystem::path& Platform::getAssetsRoot() const {
	return assetsRoot;
}
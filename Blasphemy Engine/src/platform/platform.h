#pragma once
#include "SDL3/SDL.h"
#include "volk/volk.h"
#include <filesystem>

class Platform {
	
public:

	bool init();
	void shutdown();

	void pollEvents();
	bool shouldClose() const;

	SDL_Window* getWindow() const;
	VkExtent2D getWindowExtent() const;

	float getScale() const;

	VkSurfaceKHR createVulkanSurface(VkInstance, VkAllocationCallbacks* allocator);
	void getRequiredVulkanExtensions(std::vector<const char*>& outExtensions) const;
	const std::filesystem::path& getAssetsRoot() const;

private:
	SDL_Window* window = nullptr;
	VkExtent2D windowExtent = { 1700,900 };
	float mainScale = 1.0f;

	bool windowResizeRequest = false;
	bool quit = false;

	std::filesystem::path assetsRoot;
};
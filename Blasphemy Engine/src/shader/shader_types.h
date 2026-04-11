#pragma once

#include "volk/volk.h"


#include <filesystem>
#include <vector>

struct ShaderInfo {
	std::filesystem::path relativePath;
	VkShaderStageFlagBits stage;
	std::filesystem::file_time_type lastModified;
};

struct Shader {
	std::vector<ShaderInfo> stages;
	void addStage(std::filesystem::path relativePath, VkShaderStageFlagBits stage) { stages.push_back({ relativePath, stage }); }

};
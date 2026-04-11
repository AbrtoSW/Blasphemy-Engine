#pragma once

#include "volk/volk.h"
#include "engine/EnginePaths.h"
#include "shader_util.h"

#include <filesystem>
#include <vector>


struct ShaderInfo {
	std::filesystem::path relativePath;
	VkShaderStageFlagBits stage;
	std::filesystem::file_time_type lastModified;
};

struct Shader {
	
	std::vector<ShaderInfo> stages;

	void addShader(const EnginePaths& enginePaths, std::filesystem::path relativePath, VkShaderStageFlagBits stage) {

		auto resolved = enginePaths.gameAssets / relativePath;

		auto t = ShaderUtility::getFileTimeStamp(resolved.string());
		
		stages.push_back({ relativePath, stage, t });
	
	}

};
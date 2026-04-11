#pragma once

#include "volk/volk.h"
#include "engine/EnginePaths.h"
#include "shader_util.h"

#include <iostream>
#include <filesystem>
#include <vector>


struct ShaderInfo {
	std::filesystem::path relativePath;
	VkShaderStageFlagBits stage;
	std::filesystem::file_time_type lastModified;
};

struct Shader {
	
	std::vector<ShaderInfo> stages;

	//move these functions to a .cpp

	void addShader(const EnginePaths& enginePaths, std::filesystem::path relativePath, VkShaderStageFlagBits stage) {

		auto resolved = enginePaths.gameAssets / relativePath;

		auto t = ShaderUtility::getFileTimeStamp(resolved.string());
		
		stages.push_back({ relativePath, stage, t });
	}

	std::optional<ShaderInfo> findShader(VkShaderStageFlagBits stage) {

		auto it = std::find_if(stages.begin(), stages.end(), [stage](ShaderInfo& s) {return s.stage == stage; });

		if (it == stages.end()) {
			return std::nullopt;
		}

		return *it;
	}


};
#pragma once
#include <string>
#include <filesystem>
#include "volk/volk.h"

struct ShaderFile {

	std::string relativePath;

	ShaderFile() = default;

	explicit ShaderFile(const std::string& file) : relativePath(file) {}
	
};

struct ShaderInfo {
	ShaderFile file;
	VkShaderStageFlagBits stage;
	std::filesystem::file_time_type lastModified;
};

struct Shader {
	std::vector<ShaderInfo> stages;
};
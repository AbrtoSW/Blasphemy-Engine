#pragma once
#include <filesystem>
#include <glslang/Public/ShaderLang.h>
#include "renderer/shader/glslangDefaultTBuiltinResource.h"
#include "volk/volk.h"
#include "util/utility.h"

namespace ShaderUtility {
	std::filesystem::file_time_type getFileTimeStamp(std::string_view fullPath);
	VkShaderModule compileToSPV(VkDevice device, std::string_view fullPath, EShLanguage stage, const std::filesystem::path& includeBase);
}


class RuntimeIncluder : public glslang::TShader::Includer {
public:

	explicit RuntimeIncluder(std::filesystem::path baseDir)
		: base(std::move(baseDir)) {
	}

	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t includeDepth) override;

	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t includeDepth) override;

	void releaseInclude(IncludeResult* result) override;
private:
	std::filesystem::path base;
};



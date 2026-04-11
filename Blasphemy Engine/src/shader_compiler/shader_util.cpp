#include "shader_compiler/shader_util.h"
#include "util/vk_util.h"
#include <glslang/SPIRV/GlslangToSpv.h>
#include <fstream>
#include <iostream>

std::filesystem::file_time_type ShaderUtility::getFileTimeStamp(std::string_view fullPath) {
	return std::filesystem::last_write_time(std::filesystem::path(fullPath));
}

VkShaderModule ShaderUtility::compileToSPV(VkDevice device,std::string_view fullPath, EShLanguage stage, const std::filesystem::path& includeBase) {
	// if it's a precompiled SPIR-V blob, load it directly

	std::filesystem::path full = fullPath;

	auto ends_with_spv = [](std::string_view s) {
		if (s.size() < 4) return false;

		char a = s[s.size() - 4], b = s[s.size() - 3], c = s[s.size() - 2], d = s[s.size() - 1];
		// case-insensitive compare to ".spv"
		auto lower = [](char ch) { return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch; };
		return lower(a) == '.' && lower(b) == 's' && lower(c) == 'p' && lower(d) == 'v';
		};

	if (ends_with_spv(fullPath)) {
		// read binary file
		std::vector<char> bytes;
		{
			std::ifstream f(full, std::ios::binary | std::ios::ate);
			if (!f) {
				throw std::runtime_error("Failed to open SPIR-V file: " + full.string());
			}
			std::streamsize sz = f.tellg();
			if (sz <= 0) {
				throw std::runtime_error("Empty SPIR-V file: " + full.string());
			}
			bytes.resize(static_cast<size_t>(sz));
			f.seekg(0, std::ios::beg);
			if (!f.read(bytes.data(), sz)) {
				throw std::runtime_error("Failed to read SPIR-V file: " + full.string());
			}
		}

		if ((bytes.size() % 4) != 0) {
			throw std::runtime_error("SPIR-V size not multiple of 4: " + full.string());
		}

		VkShaderModuleCreateInfo ci{ };
		ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ci.codeSize = bytes.size();
		ci.pCode = (const uint32_t*)bytes.data();

		VkShaderModule mod = VK_NULL_HANDLE;
		if (vkCreateShaderModule(device, &ci, nullptr, &mod) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module from SPIR-V: " + full.string());
		}
		return mod;
	}

	// else: treat as GLSL source and compile at runtime (your existing path)
	std::string source = GeneralUtility::readFile(full.string());
	const char* sourcePtr = source.c_str();

	EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

	glslang::TShader shader(stage);
	shader.setStrings(&sourcePtr, 1);

	RuntimeIncluder includer(includeBase);

	if (!shader.parse(&DefaultTBuiltInResource, 110, false, messages, includer)) {
		std::cout << "Shader compile error: " << shader.getInfoLog() << "\n";
		throw std::runtime_error(shader.getInfoLog());
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messages)) {
		throw std::runtime_error(program.getInfoLog());
	}

	std::vector<uint32_t> spirv;
	GlslangToSpv(*program.getIntermediate(stage), spirv);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = spirv.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}

	return shaderModule;
}


glslang::TShader::Includer::IncludeResult* RuntimeIncluder::includeLocal(const char* headerName, const char* includerName, size_t includeDepth) {

	auto full = base / headerName;
	std::ifstream file(full);

	if (!file.is_open()) {
		std::cout << "Failed to open include file: " << headerName << "\n";
		return nullptr;
	}

	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Allocate heap memory for glslang
	char* buffer = new char[contents.size() + 1];
	memcpy(buffer, contents.c_str(), contents.size() + 1);

	return new IncludeResult(headerName, buffer, contents.size(), nullptr);
}

glslang::TShader::Includer::IncludeResult* RuntimeIncluder::includeSystem(const char* headerName, const char* includerName, size_t includeDepth) {
	return includeLocal(headerName, includerName, includeDepth);
}


void RuntimeIncluder::releaseInclude(IncludeResult* result)
{
	if (!result) return;
	delete[] result->headerData;
	delete result;
}


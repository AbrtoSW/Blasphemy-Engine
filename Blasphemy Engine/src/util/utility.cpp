#include "utility.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string GeneralUtility::readFile(const std::string& filepath) {
		std::cout << "READFILE PATH = " << filepath;

		std::ifstream file(filepath, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filepath);
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
}

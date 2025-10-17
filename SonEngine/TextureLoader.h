#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>

struct TextureInfo {
	uint64_t offset;
	uint64_t size;
};

class TextureLoader {
public:
	TextureLoader(std::string path);

	void LoadIdx();

private:
	std::string folder;
	std::string binPath;
	std::string idxPath;

	uint32_t count;

	std::unordered_map<std::string, TextureInfo> textureMap;
};
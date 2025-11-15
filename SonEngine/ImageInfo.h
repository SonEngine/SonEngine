#pragma once

#include <string>

struct ImageInfo {
	std::string name;
	long long bufferPixelCount;
	long long numRows;
	long long rowSize;
	long long rowPitch;
	long long width;
	long long height;
};
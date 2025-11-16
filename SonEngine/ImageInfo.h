#pragma once

#include <string>

struct ImageInfo {
	std::string name = "";
	long long bufferPixelCount = 0;
	long long numRows = 0;
	long long rowSize = 0;
	long long rowPitch = 0;
	long long width = 0;
	long long height = 0;
};
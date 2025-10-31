#pragma once

#include "StaticMesh.h"

struct Proxy {
	StaticMesh* mesh = nullptr;
	UINT id = 0;
};

struct TextProxy {
	StaticMesh* mesh = nullptr;
	std::wstring str = L"hello";
};
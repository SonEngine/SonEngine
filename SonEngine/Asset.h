#pragma once

#include <vector>
#include <string>
#include "Mesh.h"

template<typename V, typename I>
class Asset {
public:
	Asset() {};
	virtual ~Asset() {};

	std::vector<Mesh<V, I>> m_meshes;
	std::string name;

};
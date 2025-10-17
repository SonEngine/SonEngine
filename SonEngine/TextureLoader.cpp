#include "TextureLoader.h"

namespace fs = std::filesystem;


std::ostream& operator<<(std::ostream& out, const TextureInfo& info)
{
	out  << ", Offset : " << info.offset << " , size : " << info.size;
	out << '\n';

	return out;
}

TextureLoader::TextureLoader(std::string path)
	:folder(path)
{
	count = 0;
	binPath = folder + "/textures.bin";
	idxPath = folder + "/textures.idx";
}

void TextureLoader::LoadIdx()
{
	std::ifstream idx(idxPath.c_str(), std::ios::binary);

	idx.read(reinterpret_cast<char*>(&count), sizeof(count));

	std::cout << "dds count : " << count << '\n';

	for (uint32_t i = 0; i < count; i++)
	{
		TextureInfo info;
		uint32_t nameLen;
		
		idx.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		std::string fileName(nameLen, ' ');

		idx.read(fileName.data(), nameLen);
		idx.read(reinterpret_cast<char*>(&info.offset), sizeof(info.offset));
		idx.read(reinterpret_cast<char*>(&info.size), sizeof(info.size));

		std::cout << "File name : " << fileName << info;
		textureMap[fileName] = info;
	}
}

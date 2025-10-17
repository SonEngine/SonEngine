#include <fstream>
#include <filesystem>
#include <vector>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

struct TextureInfo {
    std::string name;
    uint64_t offset;
    uint64_t size;
};
int main()
{
    std::string inputFolder = "../SonEngine/Textures";
    std::string outputBin = "../SonEngine/Build/textures.bin";
    std::string outputIdx = "../SonEngine/Build/textures.idx";

    fs::create_directories("../SonEngine/Build");

    std::vector<TextureInfo> infos;
    std::ofstream bin(outputBin, std::ios::binary);
    std::ofstream idx(outputIdx, std::ios::binary);
    
    uint64_t offset = 0;

    for (auto& entry : fs::directory_iterator(inputFolder))
    {
        if (entry.path().extension() == ".dds")
        {
            std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
            TextureInfo info;
            auto size = file.tellg();

            info.name = entry.path().stem().string();
            info.offset = offset;
            info.size = size;

            infos.push_back(info);

            std::vector<char> texture(size);
            file.read(texture.data(), size);

            bin.write(texture.data(), size);

            offset += size;
            std::string sizeStr;
            if (size >= 1e12)
                sizeStr = std::to_string(size / (int)1e12) + " T";
            else if (size >= 1e9)
                sizeStr = std::to_string(size / (int)1e12) + " G";
            else if (size >= 1e6)
                sizeStr = std::to_string(size / (int)1e6) + " M";
            else if (size >= 1e3)
                sizeStr = std::to_string(size / (int)1e3) + " K";

            std::cout << "packed : " << info.name << " ( size : " << sizeStr << "B )\n";
        }
    }

    uint32_t count = static_cast<uint32_t>(infos.size());;

    idx.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (auto& info : infos)
    {
        uint32_t nameLen = static_cast<uint32_t>(info.name.size());
        idx.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        idx.write(info.name.data(), nameLen);
        idx.write(reinterpret_cast<const char*>(&info.offset), sizeof(info.offset));
        idx.write(reinterpret_cast<const char*>(&info.size), sizeof(info.size));
    }

    std::cout << "Done. Packed " << infos.size() << " DDS textures.\n";
}
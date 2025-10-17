#include <DirectXTex.h>
#include <filesystem>
#include <iostream>

using namespace DirectX;
namespace fs = std::filesystem;

int main()
{
    std::wstring inputPath = L"../SonEngine/Textures/earth_Albedo.dds";
    std::wstring outputPath = L"Build/earth_Albedo_fallback.dds";

    fs::create_directories("../SonEngine/Build");

    ScratchImage srcImage;
    HRESULT hr = LoadFromDDSFile(inputPath.c_str(), DDS_FLAGS_NONE, nullptr, srcImage);
    if (FAILED(hr))
        return -1;

    const TexMetadata& meta = srcImage.GetMetadata();
    size_t mipCount = meta.mipLevels;

    if (mipCount <= 1)
        return 0;

    const Image* lastMip = srcImage.GetImage(mipCount - 1, 0, 0);

    ScratchImage fallback;
    hr = fallback.Initialize2D(meta.format, lastMip->width, lastMip->height, 1, 1);
    if (FAILED(hr))
        return -1;

    memcpy(fallback.GetImages()->pixels, lastMip->pixels, lastMip->slicePitch);

    hr = SaveToDDSFile(
        fallback.GetImages(),
        fallback.GetImageCount(),
        fallback.GetMetadata(),
        DDS_FLAGS_NONE,
        outputPath.c_str()
    );

    if (FAILED(hr))
        return -1;

    std::wcout << L"Fallback DDS created: " << outputPath << std::endl;
}

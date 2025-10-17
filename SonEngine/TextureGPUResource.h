#pragma once

#include "wrl.h"
#include "d3d12.h"

struct TextureGPUResource
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
};
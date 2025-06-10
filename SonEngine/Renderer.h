#pragma once

#include "TextureManager.h"
#include <vector>

class GraphicsPSO;
class RootSignature;
class DescriptorHeap;
class DescriptorHandle;


namespace Renderer
{

    extern std::vector<GraphicsPSO> sm_PSOs;
    extern RootSignature m_RootSig;
    extern DescriptorHeap s_TextureHeap;
    extern DescriptorHeap s_SamplerHeap;
    extern DescriptorHandle m_CommonTextures;

    void Initialize(void);
    void Shutdown(void);

    uint8_t GetPSO(uint16_t psoFlags);


} // namespace Renderer
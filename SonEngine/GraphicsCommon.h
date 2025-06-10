#pragma once

#include "Utility.h"

class SamplerDesc;
class RootSignature;
class ComputePSO;
class GraphicsPSO;

namespace Graphics
{
    void InitializeCommonState(Microsoft::WRL::ComPtr<ID3D12Device5> & device);


    extern D3D12_SAMPLER_DESC SamplerLinearWrapDesc;
  
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;

    extern D3D12_RASTERIZER_DESC RasterizerDefault;

    extern D3D12_BLEND_DESC BlendNoColorWrite;		// XXX

    extern D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;

    extern RootSignature g_CommonRS;
}

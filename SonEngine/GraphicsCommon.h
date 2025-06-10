#pragma once

#include "Utility.h"

class SamplerDesc;
class RootSignature;
class ComputePSO;
class GraphicsPSO;

namespace Graphics
{
    // sampler, rootsignature 등을 미리 생성
    void InitializeCommonState(const Microsoft::WRL::ComPtr<ID3D12Device5> & device);

    extern D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;
  
    extern D3D12_RASTERIZER_DESC rasterizerDefault;

    extern D3D12_BLEND_DESC blendNoColorWrite;		

    extern D3D12_DEPTH_STENCIL_DESC depthStateDisabled;

    extern RootSignature g_commonRS;
}

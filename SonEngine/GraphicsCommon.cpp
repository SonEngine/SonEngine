#include "GraphicsCommon.h"
#include "PipelineState.h"
#include "RootSignature.h"

namespace Graphics
{
  
    D3D12_SAMPLER_DESC SamplerLinearWrapDesc;

    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;

    D3D12_RASTERIZER_DESC RasterizerDefault;

    D3D12_BLEND_DESC BlendNoColorWrite;		// XXX

    D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;

    RootSignature g_CommonRS;
}


void Graphics::InitializeCommonState(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
    SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearWrapDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerLinearWrapDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerLinearWrapDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerLinearWrapDesc.MipLODBias = 0.0f;
    SamplerLinearWrapDesc.MaxAnisotropy = 16;
    SamplerLinearWrapDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    SamplerLinearWrapDesc.BorderColor[0] = 1.0f;
    SamplerLinearWrapDesc.BorderColor[1] = 1.0f;
    SamplerLinearWrapDesc.BorderColor[2] = 1.0f;
    SamplerLinearWrapDesc.BorderColor[3] = 1.0f;
    SamplerLinearWrapDesc.MinLOD = 0.0f;
    SamplerLinearWrapDesc.MaxLOD = D3D12_FLOAT32_MAX;
   
    device->CreateSampler(&SamplerLinearWrapDesc, SamplerLinearWrap);
    

}


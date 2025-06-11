#include "GraphicsCommon.h"
#include "PipelineState.h"
#include "RootSignature.h"

namespace Graphics
{
  
	D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;

    D3D12_RASTERIZER_DESC rasterizerDefault;

    D3D12_BLEND_DESC blendNoColorWrite;		

    D3D12_DEPTH_STENCIL_DESC depthStateDisabled;

    RootSignature g_commonRS;
}


void Graphics::InitializeCommonState(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	wrapLinearSampler = {};
	wrapLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	wrapLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.MipLODBias = 0;
	wrapLinearSampler.MaxAnisotropy = 0;
	wrapLinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	wrapLinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	wrapLinearSampler.MinLOD = 0.0f;
	wrapLinearSampler.MaxLOD = D3D12_FLOAT32_MAX;
	wrapLinearSampler.ShaderRegister = 0;
	wrapLinearSampler.RegisterSpace = 0;
	wrapLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
   
	rasterizerDefault = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	blendNoColorWrite = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	depthStateDisabled = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    g_commonRS.Reset(1, 0);
    g_commonRS[0].InitCBV();
	//g_commonRS.InitStaticSampler(0, wrapLinearSampler);

	g_commonRS.Finalize(device, L"CommonRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}


#include "GraphicsCommon.h"
#include "PipelineState.h"
#include "RootSignature.h"

namespace Graphics
{
  
	D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;

	D3D12_RASTERIZER_DESC rasterizerDefault;
	D3D12_RASTERIZER_DESC noneCullRasterizer;

    D3D12_BLEND_DESC blendNoColorWrite;		

    D3D12_DEPTH_STENCIL_DESC depthStateDefault;

	RootSignature g_commonRS;
	RootSignature g_pointCloudRS;
	RootSignature g_cubeMapRS;
	RootSignature g_videoRS;
	RootSignature g_U1_C1_RS;
	RootSignature g_R1_RS;
	RootSignature g_R2_C2_RS;

	std::shared_ptr<GraphicsUtils::Utility> utility;
	std::unique_ptr<World> world;
	std::mutex g_imguiMutex;
	std::atomic<bool> g_imguiReady{ false };
	ImGuiContext* g_imguiCtx = nullptr;
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
	noneCullRasterizer = rasterizerDefault;
	noneCullRasterizer.CullMode = D3D12_CULL_MODE_NONE;
	noneCullRasterizer.DepthClipEnable = false;

	blendNoColorWrite = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	depthStateDefault = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	
	g_U1_C1_RS.Reset(2, 1);
	g_U1_C1_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	g_U1_C1_RS[1].InitCBV(0);
	g_U1_C1_RS.InitStaticSampler(0, wrapLinearSampler);
	g_U1_C1_RS.Finalize(device, L"U1_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    g_commonRS.Reset(3, 1);
	g_commonRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_commonRS[1].InitCBV();
	g_commonRS[2].InitCBV(1);
	g_commonRS.InitStaticSampler(0, wrapLinearSampler);
	g_commonRS.Finalize(device, L"CommonRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_videoRS.Reset(3, 1);
	g_videoRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	g_videoRS[1].InitCBV();
	g_videoRS[2].InitCBV(1);
	g_videoRS.InitStaticSampler(0, wrapLinearSampler);
	g_videoRS.Finalize(device, L"VideoRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_pointCloudRS.Reset(2);
	g_pointCloudRS[0].InitCBV(0);
	g_pointCloudRS[1].InitCBV(1);
	g_pointCloudRS.Finalize(device, L"PointCloudRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_R1_RS.Reset(1, 1);
	g_R1_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_R1_RS.InitStaticSampler(0, wrapLinearSampler);
	g_R1_RS.Finalize(device, L"R1_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_cubeMapRS.Reset(2, 1);
	g_cubeMapRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_cubeMapRS[1].InitCBV(0);
	g_cubeMapRS.InitStaticSampler(0, wrapLinearSampler);
	g_cubeMapRS.Finalize(device, L"cubeMapRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_R2_C2_RS.Reset(4, 1);
	g_R2_C2_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3); // cubemap
	g_R2_C2_RS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1); // albedo
	g_R2_C2_RS[2].InitCBV(0); // local
	g_R2_C2_RS[3].InitCBV(1); // global
	g_R2_C2_RS.InitStaticSampler(0, wrapLinearSampler);
	g_R2_C2_RS.Finalize(device, L"R2_C2_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

}


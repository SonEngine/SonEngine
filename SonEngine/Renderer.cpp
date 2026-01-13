#include "Renderer.h"
#include "RootSignature.h"
#include "PipelineState.h"

#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultVS.h"
#include "CompiledShaders/PaintBoardCS.h"

#include "CompiledShaders/VideoPS.h"

#include "CompiledShaders/PhongVS.h"
#include "CompiledShaders/PhongPS.h"

#include "CompiledShaders/PBRVS.h"
#include "CompiledShaders/PBRPS.h"

#include "CompiledShaders/PointCloudVS.h"
#include "CompiledShaders/PointCloudGS.h"
#include "CompiledShaders/PointCloudPS.h"

#include "CompiledShaders/CubeMapVS.h"
#include "CompiledShaders/CubeMapPS.h"

#include "CompiledShaders/BoxCubeMapVS.h"
#include "CompiledShaders/BoxCubeMapPS.h"

#include "CompiledShaders/PBRBoxCubeMapVS.h"
#include "CompiledShaders/PBRBoxCubeMapPS.h"

#include "CompiledShaders/DepthOnlyVS.h"
#include "CompiledShaders/DepthOnlyPS.h"

#include "CompiledShaders/RenderTextureVS.h"
#include "CompiledShaders/RenderTextureGS.h"
#include "CompiledShaders/RenderTexturePS.h"

#include "CompiledShaders/TextVS.h"
#include "CompiledShaders/TextPS.h"

using namespace Graphics;
using namespace Renderer;


namespace Renderer
{
	std::map<std::string, GraphicsPSO> m_PSOs;
	std::map<std::string, ComputePSO> m_CPSOs;

	std::vector<std::string> psoNames;
	std::vector<std::string> cpsoNames;

	DXGI_FORMAT hdrFormat;
	DXGI_FORMAT backBufferFormat;
	DXGI_FORMAT dsBufferFormat;
	DXGI_FORMAT dsOnlyFormat;
	DXGI_FORMAT dsOnlyDsvFormat;
	DXGI_FORMAT dsOnlySrvFormat;

}

void Renderer::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	GraphicsPSO defaultPSO(L"default PSO");
	GraphicsPSO videoPSO(L"video PSO");
	GraphicsPSO phongPSO(L"phong PSO");
	GraphicsPSO pbrPSO(L"pbr PSO");
	GraphicsPSO dsOnlyPbrPSO(L"dsOnlyPbr PSO");
	GraphicsPSO wirePbrPSO(L"wirePbr PSO");

	GraphicsPSO textPSO(L"text PSO");

	GraphicsPSO pointCloudPSO(L"pointCloud PSO");
	GraphicsPSO renderTexturePSO(L"renderTexture PSO");
	GraphicsPSO cubeMapPSO(L"cubeMap PSO");
	GraphicsPSO cubeMapCullPSO(L"cubeMapCull PSO");
	GraphicsPSO genCubeMapPSO(L"genCubeMap PSO");
	GraphicsPSO genPBRCubeMapPSO(L"genPBRCubeMap PSO");

	ComputePSO defaultCPSO(L"default CPSO");

	hdrFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//backBufferFormat  = hdrFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsOnlyFormat = DXGI_FORMAT_R32_TYPELESS;
	dsOnlyDsvFormat = DXGI_FORMAT_D32_FLOAT;
	dsOnlySrvFormat = DXGI_FORMAT_R32_FLOAT;

	D3D12_INPUT_ELEMENT_DESC posOnlyIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC simpleIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}

	};
	D3D12_INPUT_ELEMENT_DESC textIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC phongIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC pbrIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	D3D12_INPUT_ELEMENT_DESC pointCloudIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	defaultPSO.SetInputLayout(_countof(simpleIL), simpleIL);
	defaultPSO.SetRootSignature(g_commonRS);
	defaultPSO.SetRasterizerState(rasterizerDefault);
	defaultPSO.SetBlendState(blendNoColorWrite);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	defaultPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
	defaultPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
	defaultPSO.SetSampleMask(UINT_MAX);
	defaultPSO.SetRenderTargetFormat(backBufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
	defaultPSO.Finalize(device);
	m_PSOs["defaultPSO"] = defaultPSO;
	psoNames.push_back("defaultPSO");

	videoPSO.SetInputLayout(_countof(simpleIL), simpleIL);
	videoPSO.SetRootSignature(g_videoRS);
	videoPSO.SetRasterizerState(rasterizerDefault);
	videoPSO.SetBlendState(blendNoColorWrite);
	videoPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	videoPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
	videoPSO.SetPixelShader(g_pVideoPS, sizeof(g_pVideoPS));
	videoPSO.SetSampleMask(UINT_MAX);
	videoPSO.SetRenderTargetFormat(backBufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
	videoPSO.Finalize(device);
	m_PSOs["videoPSO"] = videoPSO;

	// BOOKMARK
	phongPSO.SetInputLayout(_countof(phongIL), phongIL);
	phongPSO.SetRootSignature(g_R4_C2_RS);
	phongPSO.SetRasterizerState(rasterizerDefault);
	phongPSO.SetBlendState(blendNoColorWrite);
	phongPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	phongPSO.SetVertexShader(g_pPhongVS, sizeof(g_pPhongVS));
	phongPSO.SetPixelShader(g_pPhongPS, sizeof(g_pPhongPS));
	phongPSO.SetSampleMask(UINT_MAX);
	phongPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	phongPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	phongPSO.SetDepthStencilState(depthStateDefault);
	phongPSO.Finalize(device);
	m_PSOs["phongPSO"] = phongPSO;
	psoNames.push_back("phongPSO");

	pbrPSO.SetInputLayout(_countof(pbrIL), pbrIL);
	pbrPSO.SetRootSignature(g_R4_C2_RS);
	pbrPSO.SetRasterizerState(rasterizerDefault);
	pbrPSO.SetBlendState(blendNoColorWrite);
	pbrPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pbrPSO.SetVertexShader(g_pPBRVS, sizeof(g_pPBRVS));
	pbrPSO.SetPixelShader(g_pPBRPS, sizeof(g_pPBRPS));
	pbrPSO.SetSampleMask(UINT_MAX);
	pbrPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	pbrPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	pbrPSO.SetDepthStencilState(depthStateDefault);
	pbrPSO.Finalize(device);
	m_PSOs["pbrPSO"] = pbrPSO;
	psoNames.push_back("pbrPSO");

	wirePbrPSO.SetInputLayout(_countof(pbrIL), pbrIL);
	wirePbrPSO.SetRootSignature(g_R4_C2_RS);
	wirePbrPSO.SetRasterizerState(wireRasterizer);
	wirePbrPSO.SetBlendState(blendNoColorWrite);
	wirePbrPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	wirePbrPSO.SetVertexShader(g_pPBRVS, sizeof(g_pPBRVS));
	wirePbrPSO.SetPixelShader(g_pPBRPS, sizeof(g_pPBRPS));
	wirePbrPSO.SetSampleMask(UINT_MAX);
	wirePbrPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	wirePbrPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	wirePbrPSO.SetDepthStencilState(depthStateDefault);
	wirePbrPSO.Finalize(device);
	m_PSOs["wire_pbrPSO"] = wirePbrPSO;
	psoNames.push_back("wire_pbrPSO");

	dsOnlyPbrPSO.SetInputLayout(_countof(pbrIL), pbrIL);
	dsOnlyPbrPSO.SetRootSignature(g_R1_C2_RS);
	dsOnlyPbrPSO.SetRasterizerState(rasterizerDefault);
	dsOnlyPbrPSO.SetBlendState(blendNoColorWrite);
	dsOnlyPbrPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	dsOnlyPbrPSO.SetVertexShader(g_pDepthOnlyVS, sizeof(g_pDepthOnlyVS));
	dsOnlyPbrPSO.SetPixelShader(g_pDepthOnlyPS, sizeof(g_pDepthOnlyPS));
	dsOnlyPbrPSO.SetSampleMask(UINT_MAX);
	//dsOnlyPbrPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	dsOnlyPbrPSO.SetDepthTargetFormat(dsOnlyDsvFormat, 1, 0);
	dsOnlyPbrPSO.SetDepthStencilState(depthStateDefault);
	dsOnlyPbrPSO.Finalize(device);
	m_PSOs["dsOnly_pbrPSO"] = dsOnlyPbrPSO;
	psoNames.push_back("dsOnly_pbrPSO");

	textPSO.SetInputLayout(_countof(textIL), textIL);
	textPSO.SetRootSignature(g_commonRS);
	textPSO.SetRasterizerState(rasterizerDefault);
	textPSO.SetBlendState(blendNoColorWrite);
	textPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	textPSO.SetVertexShader(g_pTextVS, sizeof(g_pTextVS));
	textPSO.SetPixelShader(g_pTextPS, sizeof(g_pTextPS));
	textPSO.SetSampleMask(UINT_MAX);
	textPSO.SetRenderTargetFormat(backBufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
	textPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	textPSO.SetDepthStencilState(depthStateDefault);
	textPSO.Finalize(device);
	m_PSOs["textPSO"] = textPSO;
	psoNames.push_back("textPSO");

	pointCloudPSO.SetInputLayout(_countof(pointCloudIL), pointCloudIL);
	pointCloudPSO.SetRootSignature(g_pointCloudRS);
	pointCloudPSO.SetRasterizerState(noneCullRasterizer);
	pointCloudPSO.SetBlendState(blendNoColorWrite);
	pointCloudPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	pointCloudPSO.SetVertexShader(g_pPointCloudVS, sizeof(g_pPointCloudVS));
	pointCloudPSO.SetGeometryShader(g_pPointCloudGS, sizeof(g_pPointCloudGS));
	pointCloudPSO.SetPixelShader(g_pPointCloudPS, sizeof(g_pPointCloudPS));
	pointCloudPSO.SetSampleMask(UINT_MAX);
	pointCloudPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	pointCloudPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	pointCloudPSO.SetDepthStencilState(depthStateDefault);
	pointCloudPSO.Finalize(device);
	m_PSOs["pointCloudPSO"] = pointCloudPSO;
	psoNames.push_back("pointCloudPSO");

	renderTexturePSO.SetInputLayout(_countof(pointCloudIL), pointCloudIL);
	renderTexturePSO.SetRootSignature(g_R1_RS);
	renderTexturePSO.SetRasterizerState(noneCullRasterizer);
	renderTexturePSO.SetBlendState(blendNoColorWrite);
	renderTexturePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	renderTexturePSO.SetVertexShader(g_pRenderTextureVS, sizeof(g_pRenderTextureVS));
	renderTexturePSO.SetGeometryShader(g_pRenderTextureGS, sizeof(g_pRenderTextureGS));
	renderTexturePSO.SetPixelShader(g_pRenderTexturePS, sizeof(g_pRenderTexturePS));
	renderTexturePSO.SetSampleMask(UINT_MAX);
	renderTexturePSO.SetRenderTargetFormat(backBufferFormat, dsBufferFormat, 1, 0);
	renderTexturePSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	renderTexturePSO.SetDepthStencilState(depthStateDefault);
	renderTexturePSO.Finalize(device);
	m_PSOs["renderTexturePSO"] = renderTexturePSO;
	psoNames.push_back("renderTexturePSO");

	cubeMapPSO.SetInputLayout(_countof(simpleIL), simpleIL);
	cubeMapPSO.SetRootSignature(g_cubeMapRS);
	cubeMapPSO.SetRasterizerState(noneCullRasterizer);
	cubeMapPSO.SetBlendState(blendNoColorWrite);
	cubeMapPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	cubeMapPSO.SetVertexShader(g_pCubeMapVS, sizeof(g_pCubeMapVS));
	cubeMapPSO.SetPixelShader(g_pCubeMapPS, sizeof(g_pCubeMapPS));
	cubeMapPSO.SetSampleMask(UINT_MAX);
	cubeMapPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	cubeMapPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	cubeMapPSO.SetDepthStencilState(depthStateDefault);
	cubeMapPSO.Finalize(device);
	m_PSOs["cubeMapPSO"] = cubeMapPSO;
	psoNames.push_back("cubeMapPSO");

	cubeMapCullPSO.SetInputLayout(_countof(simpleIL), simpleIL);
	cubeMapCullPSO.SetRootSignature(g_cubeMapRS);
	cubeMapCullPSO.SetRasterizerState(rasterizerDefault);
	cubeMapCullPSO.SetBlendState(blendNoColorWrite);
	cubeMapCullPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	cubeMapCullPSO.SetVertexShader(g_pCubeMapVS, sizeof(g_pCubeMapVS));
	cubeMapCullPSO.SetPixelShader(g_pCubeMapPS, sizeof(g_pCubeMapPS));
	cubeMapCullPSO.SetSampleMask(UINT_MAX);
	cubeMapCullPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	cubeMapCullPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	cubeMapCullPSO.SetDepthStencilState(depthStateDefault);
	cubeMapCullPSO.Finalize(device);
	m_PSOs["cubeMapCullPSO"] = cubeMapCullPSO;
	psoNames.push_back("cubeMapCullPSO");

	genCubeMapPSO.SetInputLayout(_countof(phongIL), phongIL);
	genCubeMapPSO.SetRootSignature(g_R2_C2_RS);
	genCubeMapPSO.SetRasterizerState(rasterizerDefault);
	genCubeMapPSO.SetBlendState(blendNoColorWrite);
	genCubeMapPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	genCubeMapPSO.SetVertexShader(g_pBoxCubeMapVS, sizeof(g_pBoxCubeMapVS));
	genCubeMapPSO.SetPixelShader(g_pBoxCubeMapPS, sizeof(g_pBoxCubeMapPS));
	genCubeMapPSO.SetSampleMask(UINT_MAX);
	genCubeMapPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	genCubeMapPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	genCubeMapPSO.SetDepthStencilState(depthStateDefault);
	genCubeMapPSO.Finalize(device);
	m_PSOs["genCubeMapPSO"] = genCubeMapPSO;
	psoNames.push_back("genCubeMapPSO");

	genPBRCubeMapPSO.SetInputLayout(_countof(pbrIL), pbrIL);
	genPBRCubeMapPSO.SetRootSignature(g_R2_C2_RS);
	genPBRCubeMapPSO.SetRasterizerState(rasterizerDefault);
	genPBRCubeMapPSO.SetBlendState(blendNoColorWrite);
	genPBRCubeMapPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	genPBRCubeMapPSO.SetVertexShader(g_pPBRBoxCubeMapVS, sizeof(g_pPBRBoxCubeMapVS));
	genPBRCubeMapPSO.SetPixelShader(g_pPBRBoxCubeMapPS, sizeof(g_pPBRBoxCubeMapPS));
	genPBRCubeMapPSO.SetSampleMask(UINT_MAX);
	genPBRCubeMapPSO.SetRenderTargetFormat(hdrFormat, dsBufferFormat, 1, 0);
	genPBRCubeMapPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
	genPBRCubeMapPSO.SetDepthStencilState(depthStateDefault);
	genPBRCubeMapPSO.Finalize(device);
	m_PSOs["genPBRCubeMapPSO"] = genPBRCubeMapPSO;
	psoNames.push_back("genPBRCubeMapPSO");

	defaultCPSO.SetComputeShader(g_pPaintBoardCS, sizeof(g_pPaintBoardCS));
	defaultCPSO.SetRootSignature(g_U1_C1_RS);
	defaultCPSO.Finalize(device);
	m_CPSOs["defaultCPSO"] = defaultCPSO;
	cpsoNames.push_back("defaultCPSO");
}

ID3D12PipelineState* Renderer::GetPSO(std::string psoName)
{
	return m_PSOs[psoName].GetPSO();
}

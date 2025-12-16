#include "Renderer.h"
#include "RootSignature.h"
#include "PipelineState.h"

#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultVS.h"
#include "CompiledShaders/DefaultCS.h"

#include "CompiledShaders/VideoPS.h"

#include "CompiledShaders/PhongVS.h"
#include "CompiledShaders/PhongPS.h"

#include "CompiledShaders/PointCloudVS.h"
#include "CompiledShaders/PointCloudGS.h"
#include "CompiledShaders/PointCloudPS.h"

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
 
    DXGI_FORMAT backBufferFormat;
    DXGI_FORMAT dsBufferFormat;
}

void Renderer::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
    GraphicsPSO defaultPSO(L"default PSO");
    GraphicsPSO videoPSO(L"video PSO");
    GraphicsPSO phongPSO(L"phong PSO");
    GraphicsPSO textPSO(L"text PSO");
    GraphicsPSO pointCloudPSO(L"pointCloud PSO");
    GraphicsPSO renderTexturePSO(L"renderTexture PSO");

    ComputePSO defaultCPSO(L"default CPSO");

    backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
    dsBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
    defaultPSO.SetPixelShader(g_pDefaultPS,sizeof(g_pDefaultPS));
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

    phongPSO.SetInputLayout(_countof(phongIL), phongIL);
    phongPSO.SetRootSignature(g_commonRS);
    phongPSO.SetRasterizerState(rasterizerDefault);
    phongPSO.SetBlendState(blendNoColorWrite);
    phongPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    phongPSO.SetVertexShader(g_pPhongVS, sizeof(g_pPhongVS));
    phongPSO.SetPixelShader(g_pPhongPS, sizeof(g_pPhongPS));
    phongPSO.SetSampleMask(UINT_MAX);
    phongPSO.SetRenderTargetFormat(backBufferFormat, dsBufferFormat, 1, 0);
    phongPSO.SetDepthTargetFormat(dsBufferFormat, 1, 0);
    phongPSO.SetDepthStencilState(depthStateDefault);
    phongPSO.Finalize(device);
    m_PSOs["phongPSO"] = phongPSO;
    psoNames.push_back("phongPSO");

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
    pointCloudPSO.SetRenderTargetFormat(backBufferFormat, dsBufferFormat, 1, 0);
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

    defaultCPSO.SetComputeShader(g_pDefaultCS, sizeof(g_pDefaultCS));
    defaultCPSO.SetRootSignature(g_U1_RS);
    defaultCPSO.Finalize(device);
    m_CPSOs["defaultCPSO"] = defaultCPSO;
    cpsoNames.push_back("defaultCPSO");


}

ID3D12PipelineState* Renderer::GetPSO(std::string psoName)
{
    return m_PSOs[psoName].GetPSO();
}

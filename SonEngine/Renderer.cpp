#include "Renderer.h"
#include "RootSignature.h"
#include "PipelineState.h"

#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultVS.h"

#include "CompiledShaders/VideoPS.h"

#include "CompiledShaders/PhongVS.h"
#include "CompiledShaders/PhongPS.h"

using namespace Graphics;
using namespace Renderer;


namespace Renderer
{
    std::map<std::string, GraphicsPSO> m_PSOs;
}

void Renderer::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
    GraphicsPSO defaultPSO(L"default PSO");
    GraphicsPSO videoPSO(L"video PSO");
    GraphicsPSO phongPSO(L"phong PSO");

    D3D12_INPUT_ELEMENT_DESC posOnlyIL[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    D3D12_INPUT_ELEMENT_DESC simpleIL[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}

    };
    defaultPSO.SetInputLayout(_countof(simpleIL), simpleIL);
    defaultPSO.SetRootSignature(g_commonRS);
    defaultPSO.SetRasterizerState(rasterizerDefault);
    defaultPSO.SetBlendState(blendNoColorWrite);
    defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    defaultPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    defaultPSO.SetPixelShader(g_pDefaultPS,sizeof(g_pDefaultPS));
    defaultPSO.SetSampleMask(UINT_MAX);
    defaultPSO.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, 1, 0);
    defaultPSO.Finalize(device);
    m_PSOs["defaultPSO"] = defaultPSO;

    videoPSO.SetInputLayout(_countof(simpleIL), simpleIL);
    videoPSO.SetRootSignature(g_videoRS);
    videoPSO.SetRasterizerState(rasterizerDefault);
    videoPSO.SetBlendState(blendNoColorWrite);
    videoPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    videoPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    videoPSO.SetPixelShader(g_pVideoPS, sizeof(g_pVideoPS));
    videoPSO.SetSampleMask(UINT_MAX);
    videoPSO.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, 1, 0);
    videoPSO.Finalize(device);
    m_PSOs["videoPSO"] = videoPSO;

    phongPSO.SetInputLayout(_countof(simpleIL), simpleIL);
    phongPSO.SetRootSignature(g_commonRS);
    phongPSO.SetRasterizerState(rasterizerDefault);
    phongPSO.SetBlendState(blendNoColorWrite);
    phongPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    phongPSO.SetVertexShader(g_pPhongVS, sizeof(g_pPhongVS));
    phongPSO.SetPixelShader(g_pPhongPS, sizeof(g_pPhongPS));
    phongPSO.SetSampleMask(UINT_MAX);
    phongPSO.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, 1, 0);
    phongPSO.Finalize(device);
    m_PSOs["phongPSO"] = phongPSO;

}

ID3D12PipelineState* Renderer::GetPSO(std::string psoName)
{
    return m_PSOs[psoName].GetPSO();
}

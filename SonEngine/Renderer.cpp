#include "Renderer.h"
#include "RootSignature.h"
#include "PipelineState.h"


#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultVS.h"

using namespace Graphics;
using namespace Renderer;


namespace Renderer
{
    std::vector<GraphicsPSO> sm_PSOs;
}

void Renderer::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
    GraphicsPSO defaultPSO(L"default PSO");

    D3D12_INPUT_ELEMENT_DESC posOnlyIL[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    defaultPSO.SetInputLayout(_countof(posOnlyIL), posOnlyIL);
    defaultPSO.SetRootSignature(g_commonRS);
    defaultPSO.SetRasterizerState(rasterizerDefault);
    defaultPSO.SetBlendState(blendNoColorWrite);
    defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    defaultPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    defaultPSO.SetPixelShader(g_pDefaultPS,sizeof(g_pDefaultPS));
    defaultPSO.SetSampleMask(UINT_MAX);
    defaultPSO.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, 1, 0);
    defaultPSO.Finalize(device);

    sm_PSOs.push_back(defaultPSO);
}

ID3D12PipelineState* Renderer::GetPSO(UINT index)
{
    return sm_PSOs[index].GetPSO();
}

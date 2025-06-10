#pragma once

#include "TextureManager.h"
#include <vector>

class GraphicsPSO;
class RootSignature;

namespace Renderer
{

    extern std::vector<GraphicsPSO> sm_PSOs;

    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device);
    void Shutdown(void);

    ID3D12PipelineState* GetPSO(UINT index);


} // namespace Renderer
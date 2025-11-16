#pragma once

#include "TextureManager.h"
#include <vector>
#include <map>
#include <string>

class GraphicsPSO;
class ComputePSO;
class RootSignature;

namespace Renderer
{    
    extern std::map<std::string, GraphicsPSO> m_PSOs;
    extern std::map<std::string, ComputePSO> m_CPSOs;

    extern std::vector<std::string> psoNames;
    extern std::vector<std::string> cpsoNames;

    extern DXGI_FORMAT backBufferFormat;
    extern DXGI_FORMAT dsBufferFormat;

    // PSO 초기화
    void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device);
    void Shutdown(void);

    ID3D12PipelineState* GetPSO(std::string psoName);


} // namespace Renderer
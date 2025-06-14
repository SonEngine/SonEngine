#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <stdint.h>
#include "Utility.h"

class GpuResource
{
public:
    GpuResource() {};
    
    ~GpuResource() { Destroy(); }

    virtual void Destroy()
    {
    }
protected:

    Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
    D3D12_RESOURCE_STATES m_UsageState;
    D3D12_RESOURCE_STATES m_TransitioningState;
    D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

    // Used to identify when a resource changes so descriptors can be copied etc.
    uint32_t m_VersionID = 0;
};

#pragma once

#include "Utility.h"

class RootParameter
{
    friend class RootSignature;
public:
    RootParameter()
    {
        m_rootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
    }

    ~RootParameter()
    {
        Clear();
    }

    void Clear()
    {
        if (m_rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            delete[] m_rootParam.DescriptorTable.pDescriptorRanges;

        m_rootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
    }

    void InitSRV(UINT numDescriptors, UINT baseShaderRegister = 0, UINT registerSpace = 0)
    {
        CD3DX12_DESCRIPTOR_RANGE1 descRange;
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numDescriptors, baseShaderRegister, registerSpace);
        m_rootParam.InitAsDescriptorTable(1, &descRange);
    }
    void InitCBV(UINT baseShaderRegister = 0, UINT registerSpace = 0)
    {
        m_rootParam.InitAsConstantBufferView(baseShaderRegister, registerSpace);
    }

protected:

    CD3DX12_ROOT_PARAMETER1 m_rootParam;
};


class RootSignature
{
public:

    RootSignature(UINT numRootParams = 0, UINT numStaticSamplers = 0) 
    {
        Reset(numRootParams, numStaticSamplers);
    }

    ~RootSignature()
    {
    }

    void Reset(UINT numRootParams, UINT numStaticSamplers = 0)
    {
        if (numRootParams > 0)
            m_paramArray.reset(new RootParameter[numRootParams]);
        else
            m_paramArray = nullptr;
        m_numParameters = numRootParams;

        if (numStaticSamplers > 0)
            m_samplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers]);
        else
            m_samplerArray = nullptr;
        m_numSamplers = numStaticSamplers;
       
    }
    RootParameter& operator[](size_t index)
    {
        ASSERT(index < m_numParameters);
        return m_paramArray[index];
    }
    const RootParameter& operator[] (size_t entryIndex) const
    {
        ASSERT(entryIndex < m_numParameters);
        return m_paramArray.get()[entryIndex];

    }
    
    void InitStaticSampler(UINT index, const D3D12_STATIC_SAMPLER_DESC& staticSamplerDesc,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    // sampler & root parameter 설정 후 Root Signature를 생성
    void Finalize(const  Microsoft::WRL::ComPtr<ID3D12Device5>& device, const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ID3D12RootSignature* GetSignature() const { return m_signature.Get(); }

protected:

    std::unique_ptr<RootParameter[]> m_paramArray;
    std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> m_samplerArray;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_signature;
    UINT m_numParameters;
    UINT m_numSamplers;
};

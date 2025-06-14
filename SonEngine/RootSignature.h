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

    void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        InitAsDescriptorTable(1, Visibility);
        SetTableRange(0, Type, Register, Count, Space);
    }

    void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        m_rootParam.ShaderVisibility = Visibility;
        m_rootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
        m_rootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
    }

    void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space = 0)
    {
        D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_rootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
        range->RangeType = Type;
        range->NumDescriptors = Count;
        range->BaseShaderRegister = Register;
        range->RegisterSpace = Space;
        range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    }
    void InitCBV(UINT baseShaderRegister = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT registerSpace = 0)
    {
        m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        m_rootParam.ShaderVisibility = visibility;
        m_rootParam.Descriptor.ShaderRegister = baseShaderRegister;
        m_rootParam.Descriptor.RegisterSpace = registerSpace;
    }

protected:
    D3D12_ROOT_PARAMETER m_rootParam;
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

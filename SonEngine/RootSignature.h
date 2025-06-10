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

    void InitDefault()
    {
        CD3DX12_ROOT_PARAMETER1 param;
        CD3DX12_DESCRIPTOR_RANGE1 descRange;
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        param.InitAsDescriptorTable(1, &descRange);
    }

protected:

    D3D12_ROOT_PARAMETER m_rootParam;
};


class RootSignature
{
public:

    RootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0) 
    {
        Reset(NumRootParams, NumStaticSamplers);
    }

    ~RootSignature()
    {
    }

    void Reset(UINT NumRootParams, UINT NumStaticSamplers = 0)
    {
        if (NumRootParams > 0)
            m_ParamArray.reset(new RootParameter[NumRootParams]);
        else
            m_ParamArray = nullptr;
        m_numParameters = NumRootParams;

       
    }
    RootParameter& operator[](size_t index)
    {
        ASSERT(index < m_numParameters);
        return m_ParamArray[index];
    }
    ID3D12RootSignature* GetSignature() const { return m_Signature; }

protected:

    std::unique_ptr<RootParameter[]> m_ParamArray;
    ID3D12RootSignature* m_Signature;
    UINT m_numParameters;
};

#include "RootSignature.h"

using Microsoft::WRL::ComPtr;

void RootSignature::InitStaticSampler(UINT index, const D3D12_STATIC_SAMPLER_DESC& staticSamplerDesc, D3D12_SHADER_VISIBILITY visibility)
{
	ASSERT(index < m_numSamplers);
	m_samplerArray[index] = staticSamplerDesc;
}

void RootSignature::Finalize(const ComPtr<ID3D12Device5> & device, const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	 
	m_rootSignatureDesc.Init_1_1(
		m_numParameters,
		(const D3D12_ROOT_PARAMETER1*)m_paramArray.get(),
		m_numSamplers,
		(const D3D12_STATIC_SAMPLER_DESC*)m_samplerArray.get(), 
		flags
	);
	
	

	ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
	
	ThrowIfFailed(D3D12SerializeVersionedRootSignature(
		&m_rootSignatureDesc, 
		pOutBlob.GetAddressOf(), 
		pErrorBlob.GetAddressOf()
		));

	ThrowIfFailed(device->CreateRootSignature(0,
		pOutBlob->GetBufferPointer(),
		pOutBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_signature)
		));

	m_signature->SetName(name.c_str());
}

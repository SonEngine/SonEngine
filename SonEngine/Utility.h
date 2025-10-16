#pragma once

#include <string>
#include <vector>
#include <memory>

#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "dxgi1_6.h"
#include <directxtk12/SimpleMath.h>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1) 

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif


#if defined (DEBUG) || (_DEBUG)

#define ASSERT( isFalse, ... ) \
        if (!(bool)(isFalse)) { \
            __debugbreak(); \
        }
#endif

inline DirectX::SimpleMath::Vector4 ToVector4(const DirectX::SimpleMath::Vector3& v, float w = 0.f)
{
	return DirectX::SimpleMath::Vector4(v.x, v.y, v.z, w);
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};


namespace GraphicsUtils {
	using namespace std;
	typedef unsigned char byte;

	typedef shared_ptr<vector<byte> > ByteArray;

	ByteArray ReadFileHelper(const wstring& fileName);

	class Utility {
	public:
		Utility();
		Utility(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList);

	private:
		ID3D12Device5* m_device;
		ID3D12GraphicsCommandList* m_commandList;

	public:
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
		void CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap, UINT nodeMask = 0, D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		void CreateConstantBuffer(UINT bufferSize, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, void** pConstant);
		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(ID3D12Resource* resource);
		template<typename Data>
		void CreateTextureBuffer(
			Data* data,
			UINT bytesPerData,
			UINT width,
			UINT height,
			Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
		);

		template<typename Data>
		void CreateBuffer(
			const std::vector<Data>& data,
			Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
		);
	};




	template<typename Data>
	inline void Utility::CreateTextureBuffer(Data* data, UINT bytesPerData, UINT width, UINT height, Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(gpuBuffer.ReleaseAndGetAddressOf())
		));

		UINT64 uploadSize = 0;
		m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadSize);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
		));
		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = data;
		subresource.RowPitch = width * bytesPerData;
		subresource.SlicePitch = subresource.RowPitch * height;

		UpdateSubresources(
			m_commandList,
			gpuBuffer.Get(),
			uploadBuffer.Get(),
			0, 0, 1, &subresource);

		//auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		//	gpuBuffer.Get(),
		//	D3D12_RESOURCE_STATE_COPY_DEST,
		//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//m_commandList->ResourceBarrier(1, &barrier);
	}

	template<typename Data>
	inline void Utility::CreateBuffer(
		const std::vector<Data> & data,
		Microsoft::WRL::ComPtr<ID3D12Resource>& gpuBuffer, 
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(gpuBuffer.GetAddressOf())
		));

		D3D12_SUBRESOURCE_DATA subData;
		subData.pData = data.data();
		subData.RowPitch = sizeof(Data) * data.size();
		subData.SlicePitch = subData.RowPitch;
		
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources(
			m_commandList,
			gpuBuffer.Get(),
			uploadBuffer.Get(),
			0, 0, 1, &subData
		);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	}

}
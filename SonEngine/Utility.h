#pragma once

#include <string>
#include <vector>
#include <memory>

#include "wrl.h"
#include "d3d12.h"
#include "directx/d3dx12.h"
#include "dxgi1_6.h"
#include <directxtk12/SimpleMath.h>

#include "Actor.h"
//#include "StaticMesh.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
template<typename V, typename I> class Mesh;

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

		template<typename V, typename I>
		std::shared_ptr<Actor> CreateActor(
			const std::string& actorname,
			Mesh<V, I>& mesh,
			const std::string& texture,
			const DirectX::SimpleMath::Vector3& location);
	};

}
#include "Utility.inl"
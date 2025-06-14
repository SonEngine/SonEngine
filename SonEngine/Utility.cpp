#include "Utility.h"
#include <comdef.h>
#include <fstream>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	Filename(filename),
	LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
	// Get the string description of the error code.
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

namespace GraphicsUtils {

	using Microsoft::WRL::ComPtr;

	Utility::Utility()
		:m_device(nullptr),
		m_commandList(nullptr)
	{
	}

	Utility::Utility(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList)
		:m_device(pDevice),
		m_commandList(pCommandList)
	{
	}

	void Utility::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{

	}

	void Utility::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, ComPtr<ID3D12DescriptorHeap>& heap, UINT nodeMask, D3D12_DESCRIPTOR_HEAP_FLAGS flag)
	{
		if (m_device == nullptr) return;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Flags = flag;
		heapDesc.NodeMask = nodeMask;
		heapDesc.NumDescriptors = numDescriptors;
		heapDesc.Type = type;

		ThrowIfFailed(m_device->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(heap.ReleaseAndGetAddressOf())
		));
	}
	ByteArray ReadFileHelper(const wstring& fileName)
	{
		ByteArray NullFile = make_shared<vector<byte> >(vector<byte>());

		struct _stat64 fileStat;
		int fileExists = _wstat64(fileName.c_str(), &fileStat);
		if (fileExists == -1)
			return NullFile;

		ifstream file(fileName, ios::in | ios::binary);
		if (!file)
			return NullFile;

		ByteArray byteArray = make_shared<vector<byte> >(fileStat.st_size);
		file.read((char*)byteArray->data(), byteArray->size());
		file.close();

		return byteArray;
	}
}

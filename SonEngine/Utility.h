#pragma once

#include <string>
#include <vector>

#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"

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


namespace Utility {
	class Utility {
	public:
		Utility();
		Utility(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList);

	private:
		ID3D12Device5* m_Device;
		ID3D12GraphicsCommandList* m_CommandList;
	};
}
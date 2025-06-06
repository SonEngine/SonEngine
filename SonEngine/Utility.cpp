#include "Utility.h"
#include <comdef.h>

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

Utility::Utility::Utility()
	:m_Device(nullptr),
	m_CommandList(nullptr)
{
}

Utility::Utility::Utility(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList)
	:m_Device(pDevice),
	m_CommandList(pCommandList)
{
}

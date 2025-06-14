#pragma once

#include "GraphicsCommon.h"

// A referenced-counted pointer to a Texture.
class TextureRef;

namespace TextureManager
{
    TextureRef LoadDDSFromFile(ID3D12CommandQueue* commandQueue, ID3D12Device5* g_device, ID3D12DescriptorHeap* heap, const std::wstring& filePath, bool sRGB);
   
}

class ManagedTexture;

class TextureRef
{
public:
    TextureRef(ManagedTexture* tex = nullptr);
    /*TextureRef(const TextureRef& ref);
    TextureRef(ManagedTexture* tex = nullptr);

    void operator=(std::nullptr_t);

    void operator=(TextureRef& rhs);*/

private:
    ManagedTexture* m_ref;
};

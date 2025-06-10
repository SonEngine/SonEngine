#pragma once

#include "GraphicsCommon.h"

// A referenced-counted pointer to a Texture.
class TextureRef;

//
// Texture file loading system.
//
// References to textures are passed around so that a texture may be shared.  When
// all references to a texture expire, the texture memory is reclaimed.
//
namespace TextureManager
{
   
}

// Forward declaration; private implementation
class ManagedTexture;

//
// A handle to a ManagedTexture.  Constructors and destructors modify the reference
// count.  When the last reference is destroyed, the TextureManager is informed that
// the texture should be deleted.
//
class TextureRef
{
};

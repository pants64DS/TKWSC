#include "SM64DS_PI.h"

//@0208f9d0
namespace Particle
{
	bool Manager::LoadTex(unsigned ov0FileID, unsigned newTexID)
	{
		if(newTexID >= PARTICLE_SYS_TRACKER->manager->numTextures)
			Crash();
		TexDef& texDef = PARTICLE_SYS_TRACKER->manager->texDefArr[newTexID];
		if(texDef.texture)
			return true;
		
		SharedFilePtr tempFilePtr;
		tempFilePtr.FromOv0ID(ov0FileID);
		texDef.texture = (Texture*)tempFilePtr.Load();

		if(!texDef.texture)
			return false;

		texDef.flags  = texDef.texture->flags;
		texDef.width  = texDef.texture->Width();
		texDef.height = texDef.texture->Height();
		
		GX::BeginLoadTex();
		texDef.texVramOffset = Texture::AllocTexVram(texDef.texture->texelArrSize, texDef.texture->Format() == 5);
		GX::LoadTex(texDef.texture->TexelArr(), texDef.texVramOffset, texDef.texture->texelArrSize);
		GX::EndLoadTex();
		GX::BeginLoadTexPltt();
		texDef.palVramOffset = Texture::AllocPalVram(texDef.texture->palleteSize, texDef.texture->Format() == 2);
		GX::LoadTexPltt(texDef.texture->PalleteColArr(), texDef.palVramOffset, texDef.texture->palleteSize);
		GX::EndLoadTexPltt();
		
		return true;
	}
}

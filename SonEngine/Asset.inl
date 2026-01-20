#pragma once

#include "Asset.h"

template<typename V, typename I>
inline SkinnedLocalConstant Asset<V, I>::Update(const int& frame, const int& clipIdx)
{
	SkinnedLocalConstant slc;

	const AnimationClip& clip = clips[clipIdx];

	int currFrame = (int)(frame / 4.f);

	for (uint32_t boneId = 0; boneId < animData.boneIdToName.size(); boneId++)
	{
		const std::vector<AnimationKey>& keys = clip.keys[boneId];
		/*int32_t parentId = boneData.boneParents[i];
		const Matrix parentTransform = parentId >= 0
			? boneData.boneTransform[parentId]
			: Matrix();

		if (clip.keys[i].size() > k)
		{
			boneData.boneTransform[i] = clip.keys[i][k].m * parentTransform;
		}
		else
		{
			boneData.boneTransform[i] = parentTransform;
		}*/
		const int parentIdx = animData.boneParents[boneId];
		const Matrix parentMatrix = parentIdx >= 0
			? animData.boneTransform[parentIdx]
			: animData.accumulatedRootTransform;
		
		AnimationKey key = keys.size() > 0
			? keys[currFrame % keys.size()]
			: AnimationKey(); 

		
		Matrix keyMat =
			Matrix::CreateScale(key.scale) *
			Matrix::CreateFromQuaternion(key.quat) *
			Matrix::CreateTranslation(key.pos);


		animData.boneTransform[boneId] = keyMat * parentMatrix;
	}
	for (uint32_t i = 0; i < animData.boneIdToName.size(); i++)
	{
		slc.boneTransform[i] =
			(animData.defaultInvTransform *
				animData.boneOffset[i] *
				animData.boneTransform[i] *
				animData.defaultTransform).Transpose();
	}
	return slc;
}

#pragma once

#include "Asset.h"

template<typename V, typename I>
inline SkinnedLocalConstant Asset<V, I>::Update(const float& frame, const int& clipIdx, bool updateRootPos)
{
	SkinnedLocalConstant slc;

	const AnimationClip& clip = clips[clipIdx];

	int currFrame = (int)(frame);

	int frame0 = (int)(frame);
	int frame1 = frame0 + 1;
	float alpha = frame - float(frame0);


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
			? keys[frame0 % keys.size()]
			: AnimationKey(); 

		AnimationKey nextKey = keys.size() > 0
			? keys[frame1 % keys.size()]
			: AnimationKey();

		if (parentIdx < 0 && !updateRootPos)
		{
			key.pos.x = key.pos.z = 0.f;
		}
		XMVECTOR t = XMVectorLerp(key.pos, nextKey.pos, alpha);
		XMVECTOR s = XMVectorLerp(key.scale, nextKey.scale, alpha);
		XMVECTOR r = XMQuaternionNormalize(XMQuaternionSlerp(key.quat, nextKey.quat, alpha));

		Matrix keyMat =
			Matrix::CreateScale(s) *
			Matrix::CreateFromQuaternion(r) *
			Matrix::CreateTranslation(t); 


		//Matrix keyMat =
		//	Matrix::CreateScale(key.scale) *
		//	Matrix::CreateFromQuaternion(key.quat) *
		//	Matrix::CreateTranslation(key.pos);
	
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

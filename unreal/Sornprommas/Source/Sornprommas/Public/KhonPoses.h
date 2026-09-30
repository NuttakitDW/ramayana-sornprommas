#pragma once

#include "CoreMinimal.h"
#include "KhonFigureComponent.h"

/**
 * Target joint rotations for a Khon figure, in radians around (right, up, back)
 * axes: +X on a limb swings it forward, +X on the chest leans it back, +Y turns
 * left, +Z moves a hanging limb toward the figure's right. Sorn::R converts.
 */
struct FKhonPose
{
	FVector HipsOffset = FVector::ZeroVector; // metres (right, up, back)
	TMap<FName, FVector> Joints;

	void Set(const TCHAR* Joint, float X, float Y, float Z) { Joints.Add(FName(Joint), FVector(X, Y, Z)); }
	FVector Get(const TCHAR* Joint) const
	{
		const FVector* V = Joints.Find(FName(Joint));
		return V ? *V : FVector::ZeroVector;
	}
};

struct FKhonPoseContext
{
	EKhonKind Kind = EKhonKind::Ling;
	float Time = 0.f;
	float Beat = 0.f;
	float Move = 0.f;
	bool bGrounded = true;
	FName Action = NAME_None;
	float U = 0.f;
	float RunPhase = 0.f;
};

namespace KhonPoses
{
	FKhonPose Compute(const FKhonPoseContext& C);
}

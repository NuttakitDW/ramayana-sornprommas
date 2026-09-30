#pragma once

#include "CoreMinimal.h"

class AActor;
class UWorld;

struct FSornStrike
{
	TWeakObjectPtr<AActor> Target;
	int32 Index = -1;
	FVector Point = FVector::ZeroVector;
	float Dist = 0.f;
};

/** Shared combat queries over the hittables registered with the game mode. All units cm. */
namespace SornCombat
{
	/** Every live hittable whose nearest hurt point lies inside a forward arc. */
	TArray<FSornStrike> Sweep(const UObject* WorldContext, const FVector& Origin, const FVector& Forward,
		float Reach, float ArcDeg, float Vertical);

	/** Nearest live hurt point within MaxDist (auto-aim / AI). Index < 0 if none. */
	FSornStrike Nearest(const UObject* WorldContext, const FVector& Origin, float MaxDist, float Vertical = 250.f);

	void ClampToArena(AActor* Actor, float Margin = 0.f);

	/** Yaw (degrees) that faces a flat direction. */
	inline float YawToward(const FVector& Dir) { return FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)); }

	inline FVector Flat(const FVector& V) { return FVector(V.X, V.Y, 0.f); }
}

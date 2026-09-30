#include "SornCombat.h"

#include "GameFramework/Actor.h"
#include "SornGameMode.h"
#include "SornHittable.h"
#include "SornTypes.h"

namespace SornCombat
{
	TArray<FSornStrike> Sweep(const UObject* WorldContext, const FVector& Origin, const FVector& Forward,
		float Reach, float ArcDeg, float Vertical)
	{
		TArray<FSornStrike> Found;
		ASornGameMode* GM = ASornGameMode::Get(WorldContext);
		if (!GM)
		{
			return Found;
		}
		const FVector Fwd = Flat(Forward).GetSafeNormal();
		const float HalfCos = FMath::Cos(FMath::DegreesToRadians(ArcDeg * 0.5f));
		for (const TWeakObjectPtr<AActor>& Weak : GM->Hittables)
		{
			AActor* A = Weak.Get();
			const ISornHittable* H = Cast<ISornHittable>(A);
			if (!H || !H->IsAlive())
			{
				continue;
			}
			FSornStrike Best;
			Best.Dist = TNumericLimits<float>::Max();
			const TArray<FVector> Points = H->HurtPoints();
			for (int32 I = 0; I < Points.Num(); ++I)
			{
				const FVector To = Points[I] - Origin;
				if (FMath::Abs(To.Z) > Vertical)
				{
					continue;
				}
				const FVector F = Flat(To);
				const float D = F.Size();
				if (D > Reach + H->HurtRadius())
				{
					continue;
				}
				if (D > 90.f && ArcDeg < 359.f && FVector::DotProduct(Fwd, F / D) < HalfCos)
				{
					continue;
				}
				if (D < Best.Dist)
				{
					Best = {A, I, Points[I], D};
				}
			}
			if (Best.Index >= 0)
			{
				Found.Add(Best);
			}
		}
		return Found;
	}

	FSornStrike Nearest(const UObject* WorldContext, const FVector& Origin, float MaxDist, float Vertical)
	{
		FSornStrike Best;
		Best.Dist = MaxDist;
		ASornGameMode* GM = ASornGameMode::Get(WorldContext);
		if (!GM)
		{
			return Best;
		}
		for (const TWeakObjectPtr<AActor>& Weak : GM->Hittables)
		{
			AActor* A = Weak.Get();
			const ISornHittable* H = Cast<ISornHittable>(A);
			if (!H || !H->IsAlive())
			{
				continue;
			}
			const TArray<FVector> Points = H->HurtPoints();
			for (int32 I = 0; I < Points.Num(); ++I)
			{
				const FVector To = Points[I] - Origin;
				if (FMath::Abs(To.Z) > Vertical)
				{
					continue;
				}
				const float D = Flat(To).Size();
				if (D < Best.Dist)
				{
					Best = {A, I, Points[I], D};
				}
			}
		}
		return Best;
	}

	void ClampToArena(AActor* Actor, float Margin)
	{
		const float R = Sorn::ArenaRadius - Margin;
		FVector P = Actor->GetActorLocation();
		const FVector2D Flat2(P.X, P.Y);
		if (Flat2.Size() > R)
		{
			const FVector2D C = Flat2.GetSafeNormal() * R;
			P.X = C.X;
			P.Y = C.Y;
			Actor->SetActorLocation(P);
		}
	}
}

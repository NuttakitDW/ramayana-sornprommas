#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SornFx.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;

/**
 * One-shot visual effects built from primitives: sparks, expanding rings,
 * damage numbers, ground telegraphs, plus hit-stop and slow motion
 * (global time dilation, driven in real time).
 */
UCLASS()
class SORNPROMMAS_API ASornFx : public AActor
{
	GENERATED_BODY()

public:
	ASornFx();
	virtual void Tick(float DeltaSeconds) override;

	void Sparks(const FVector& Pos, const FLinearColor& Color, int32 Amount = 18, float Speed = 700.f);
	void Ring(const FVector& Pos, float Radius, const FLinearColor& Color, float Duration = 0.45f);
	void Number(const FVector& Pos, float Value, const FLinearColor& Color);
	void Telegraph(const FVector& Pos, float Radius, float Duration, const FLinearColor& Color);
	void Beam(const FVector& From, const FVector& To, const FLinearColor& Color, float Duration);

	void HitStop(float Seconds);
	float SlowMo = 1.f;

private:
	struct FSpark { TWeakObjectPtr<UStaticMeshComponent> Comp; FVector Vel; float Life; };
	enum class ETimedKind : uint8 { Ring, Disc, Beam, Text };

	struct FTimed
	{
		ETimedKind Kind = ETimedKind::Ring;
		TWeakObjectPtr<USceneComponent> Comp;
		TWeakObjectPtr<UMaterialInstanceDynamic> Mat;
		FLinearColor Color;
		float Age = 0.f;
		float Duration = 1.f;
		float StartScale = 0.1f;
		float EndScale = 1.f;
		float Rise = 0.f;
		bool bFade = true;
	};

	USceneComponent* MakeRing(const FVector& Pos, float Radius, UMaterialInstanceDynamic* Mat);
	void UpdateTimed(float Dt);

	TArray<FSpark> SparkList;
	TArray<FTimed> TimedList;
	TArray<TWeakObjectPtr<UTextRenderComponent>> Texts;
	double HitStopUntil = 0.0;
};

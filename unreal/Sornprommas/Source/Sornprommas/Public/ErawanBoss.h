#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SornHittable.h"
#include "ErawanBoss.generated.h"

class UKhonFigureComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHeadBroken, int32 /*Index*/, int32 /*Remaining*/);
DECLARE_MULTICAST_DELEGATE(FOnBossDefeated);

/**
 * Boss: อินทรชิต disguised as พระอินทร์ riding the illusory ช้างเอราวัณ (the demon
 * การุณราช transformed). Stage version with three heads. Pattern: hover and circle ->
 * arrow volley (เชิดฉิ่ง) or diving charge -> kneel (heads within reach) -> rise.
 * Break all three necks, echoing ร.๒ "ง้างหักฅอพระยาเอราวรรณ".
 */
UCLASS()
class SORNPROMMAS_API AErawanBoss : public AActor, public ISornHittable
{
	GENERATED_BODY()

public:
	AErawanBoss();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaSeconds) override;

	// ISornHittable
	virtual bool IsAlive() const override { return State != TEXT("dead") && State != TEXT("idle") && State != TEXT("dying"); }
	virtual TArray<FVector> HurtPoints() const override;
	virtual float HurtRadius() const override { return 90.f; }
	virtual bool ReceiveHit(float Damage, const FVector& From, float Knock, int32 PointIndex) override;

	void Appear();
	int32 HeadsLeft() const;
	UKhonFigureComponent* Rider() const { return RiderFigure; }

	static constexpr float HeadMaxHp = 80.f;
	float HeadHp[3] = {HeadMaxHp, HeadMaxHp, HeadMaxHp};
	FName State = TEXT("idle");
	/** When false the boss only circles (used during story beats). */
	bool bActive = false;

	FOnHeadBroken OnHeadBroken;
	FOnBossDefeated OnDefeated;

private:
	void BuildModel();
	USceneComponent* BuildHead(int32 I, UMaterialInterface* White, UMaterialInterface* GoldM);
	void BuildPavilion(UMaterialInterface* GoldM, UMaterialInterface* Red);
	void SetKneel(float Amount);
	void AnimateTrunks(float Dt);

	FVector OrbitPoint() const;
	void FacePlayer(float Dt, float Rate = 4.f);
	void GoHover();
	void StartVolley();
	void ScheduleArrow(const FVector& At, float Delay);
	void ArrowLand(FVector At);
	void StartCharge();
	void Land();
	void BreakHead(int32 I);
	void Cue(const FString& Id);
	void Sfx(const FString& Kind, float Pitch, float Volume = 1.f);

	UPROPERTY() TObjectPtr<USceneComponent> Body;
	UPROPERTY() TArray<TObjectPtr<USceneComponent>> Heads;
	UPROPERTY() TArray<TObjectPtr<USceneComponent>> Legs;
	UPROPERTY() TArray<TObjectPtr<USceneComponent>> TrunkSegments;
	UPROPERTY() TObjectPtr<UKhonFigureComponent> RiderFigure;

	float T = 0.f;
	float Time = 0.f;
	float OrbitA = -UE_HALF_PI;
	int32 VolleysInRow = 0;
	FVector ChargeTo = FVector::ZeroVector;
	bool bChargeHit = false;
	float Kneel = 0.f;
	TArray<float> HeadBreakT = {-1.f, -1.f, -1.f};
};

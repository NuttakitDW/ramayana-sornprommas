#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SornHittable.h"
#include "YakshaEnemy.generated.h"

class UKhonFigureComponent;
class UTextRenderComponent;
class AYakshaEnemy;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnYakshaDied, AYakshaEnemy*);

/**
 * พลยักษ์ — demon foot soldier with a club. Chases Hanuman, waits for an attack
 * token from the director, telegraphs a raised-club windup, then strikes.
 */
UCLASS()
class SORNPROMMAS_API AYakshaEnemy : public ACharacter, public ISornHittable
{
	GENERATED_BODY()

public:
	AYakshaEnemy();

	/** Call right after spawning, before the first tick. */
	void Configure(const FLinearColor& Skin, bool bCaptain, const FString& NameTag = FString());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaSeconds) override;

	// ISornHittable
	virtual bool IsAlive() const override { return State != TEXT("dead"); }
	virtual TArray<FVector> HurtPoints() const override;
	virtual float HurtRadius() const override { return 50.f * BodyScale; }
	virtual bool ReceiveHit(float Damage, const FVector& From, float Knock, int32 PointIndex) override;

	FOnYakshaDied OnDied;

	UPROPERTY(VisibleAnywhere) TObjectPtr<UKhonFigureComponent> Figure;

private:
	FVector Chase(const FVector& Dir, float Dist);
	void TryHit(float Dist, const FVector& Dir);
	FVector Separation() const;
	bool RequestToken();
	void ReleaseToken();
	void Die();
	void Face(const FVector& Dir, float Dt, float Rate);
	void Sfx(const FString& Kind, float Pitch, float Volume = 1.f);

	float MaxHp = 45.f;
	float Hp = 45.f;
	float Speed = 360.f;
	float Damage = 10.f;
	float WindupTime = 0.6f;
	bool bPoise = false;
	float BodyScale = 1.f;

	FName State = TEXT("enter");
	float T = 0.f;
	bool bHasToken = false;
	bool bStrikeDone = false;
	float OrbitDir = 1.f;
	FVector Knock = FVector::ZeroVector;
	float DeadT = 0.f;
};

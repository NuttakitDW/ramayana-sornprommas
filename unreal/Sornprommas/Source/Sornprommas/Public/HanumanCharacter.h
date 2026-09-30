#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HanumanCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UKhonFigureComponent;
class UInputAction;
class UInputMappingContext;
class UPointLightComponent;
struct FSornStrike;

DECLARE_MULTICAST_DELEGATE(FOnHanumanDied);

struct FHanumanAttack
{
	FName Pose;
	float Len;
	float HitAt;
	float Dmg;
	float Reach;   // cm
	float Arc;     // degrees
	float Knock;   // cm/s
	float Lunge;   // cm/s
	float Shake;
};

/**
 * Player character: หนุมาน with his ตรีเพชร (trident). Light combo (swing,
 * swing, overhead slam), heavy thrust, leap + plunge, dash, and นิมิตกาย
 * (magical growth, see lore/characters/hanuman.md).
 */
UCLASS()
class SORNPROMMAS_API AHanumanCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHanumanCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	bool IsAliveHanuman() const { return State != TEXT("dead"); }
	bool TakeHit(float Dmg, const FVector& From, float Knock);

	// Cutscene API used by the battle director.
	void EnterCutscene();
	void LeaveCutscene();
	void FacePoint(const FVector& P);
	void KnockDown();
	void Revive();
	void SetPuppet(bool bOn);

	static constexpr float MaxHp = 140.f;
	float Hp = MaxHp;
	float Power = 0.f;
	float GiantLeft = 0.f;
	FName State = TEXT("normal");
	bool bInputEnabled = true;
	bool bAutobot = false;
	int32 ComboCount = 0;
	int32 MaxCombo = 0;
	int32 HitsTaken = 0;

	FOnHanumanDied OnDied;

	UPROPERTY(VisibleAnywhere) TObjectPtr<UKhonFigureComponent> Figure;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Arm;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;

	/** Optional second subject the camera frames (the boss). */
	TWeakObjectPtr<AActor> CameraFocus;
	void Shake(float Amount) { Trauma = FMath::Min(1.f, Trauma + Amount); }

private:
	void BuildInput();
	void OnMove(const FInputActionValue& V);
	void OnLook(const FInputActionValue& V);
	void OnAttack();
	void OnHeavy();
	void OnJumpPressed();
	void OnDash();
	void OnSpecial();
	void OnConfirm();
	void OnRestart();
	void OnToggleHelp();

	bool Consume(FName Action);
	void Press(FName Action);

	void TickNormal(float Dt, const FVector& Wish);
	void TickAttack(float Dt);
	void TickPlunge();
	void TickDash(float Dt);
	void TickSpecial(float Dt);
	void TickTimers(float Dt);
	void TickCamera(float Dt);
	void TickBot();
	void Decelerate(float Dt, float Rate);
	void SetHorizontalVelocity(const FVector& V);

	void StartAttack(FName Id);
	void FinishAttack();
	void ResolveHit(const FHanumanAttack& Atk);
	void ApplyStrike(const FSornStrike& S, float Dmg, float Knock, float ShakeAmt);
	void AutoFace();
	void StartPlunge();
	void StartDash(const FVector& Wish);
	void StartSpecial();
	void SetGiant(bool bOn);
	void Sfx(const FString& Kind, float Pitch = 1.f, float Volume = 1.f);
	FVector Forward() const { return GetActorForwardVector(); }
	float ScaleMul() const;

	UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
	UPROPERTY() TMap<FName, TObjectPtr<UInputAction>> Actions;
	UPROPERTY() TObjectPtr<UPointLightComponent> Aura;

	FVector2D MoveInput = FVector2D::ZeroVector;
	TSet<FName> Pressed;

	FHanumanAttack Attack;
	FName AttackId = NAME_None;
	float AttackT = 0.f;
	bool bAttackHitDone = false;
	FName Queued = NAME_None;
	float DashT = 0.f;
	float DashCd = 0.f;
	FVector DashDir = FVector::ForwardVector;
	float HurtT = 0.f;
	float InvulnT = 0.f;
	float SpecialT = 0.f;
	float ComboTimer = 0.f;
	float FacingYaw = 0.f;
	float Trauma = 0.f;
	float ArmLen = 700.f;
	FVector LookPoint = FVector::ZeroVector;
	float FigureScale = 1.f;
	float TargetFigureScale = 1.f;
};

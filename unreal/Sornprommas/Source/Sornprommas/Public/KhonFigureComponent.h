#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "KhonFigureComponent.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class AMusicDirector;

UENUM()
enum class EKhonKind : uint8 { Ling, Yak, Phra };

UENUM()
enum class EKhonCrown : uint8 { None, Hanuman, Yaksha, Phra };

UENUM()
enum class EKhonWeapon : uint8 { None, Trident, Club, Bow };

UENUM()
enum class EKhonMouth : uint8 { Calm, Open, Grin };

/** What a figure looks like: Khon character type, colours, mask and crown. */
USTRUCT()
struct FKhonSpec
{
	GENERATED_BODY()

	EKhonKind Kind = EKhonKind::Ling;
	FLinearColor Skin = FLinearColor::White;
	FLinearColor Cloth = FLinearColor::Red;
	FLinearColor Trim = FLinearColor::Red;
	EKhonCrown Crown = EKhonCrown::None;
	EKhonWeapon Weapon = EKhonWeapon::None;
	EKhonMouth Mouth = EKhonMouth::Calm;
	bool bTail = false;
	/** Scanned mask asset; replaces the primitive head and crown when it loads. */
	FSoftObjectPath Mask;
};

/**
 * A Khon-style figure built from primitive meshes with a small pivot rig animated
 * procedurally: Khon stance with the ยืดยุบ (rise-and-sink) on the music beat, a run
 * cycle, a leap pose and action poses (see KhonPoses).
 */
UCLASS()
class SORNPROMMAS_API UKhonFigureComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UKhonFigureComponent();

	void Build(const FKhonSpec& InSpec);

	/** Starts an action pose; bHold keeps the last frame until another action. */
	void Play(FName InAction, float Duration, bool bHold = false);
	void ClearAction() { Action = NAME_None; }
	void Flash(const FLinearColor& Color, float Duration = 0.12f);

	FName GetAction() const { return Action; }
	const FKhonSpec& GetSpec() const { return Spec; }
	USceneComponent* GetHandL() const { return HandL; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float MoveRatio = 0.f;
	bool bGrounded = true;
	bool bDown = false;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> TailSegments;

private:
	void BuildArm(float Side, UMaterialInterface* SkinM, UMaterialInterface* GoldM,
		TObjectPtr<USceneComponent>& OutArm, TObjectPtr<USceneComponent>& OutFore, TObjectPtr<USceneComponent>& OutHand);
	void BuildLeg(float Side, UMaterialInterface* ClothM, UMaterialInterface* GoldM, UMaterialInterface* SkinM,
		TObjectPtr<USceneComponent>& OutLeg, TObjectPtr<USceneComponent>& OutShin);
	void BuildTail(UMaterialInterface* SkinM);
	void BuildWeapon();
	void ApplyPose(float DeltaTime);
	float BeatPhase();

	FKhonSpec Spec;

	UPROPERTY() TObjectPtr<USceneComponent> RootPivot;
	UPROPERTY() TObjectPtr<USceneComponent> Hips;
	UPROPERTY() TObjectPtr<USceneComponent> Chest;
	UPROPERTY() TObjectPtr<USceneComponent> Head;
	UPROPERTY() TObjectPtr<USceneComponent> ArmL;
	UPROPERTY() TObjectPtr<USceneComponent> ArmR;
	UPROPERTY() TObjectPtr<USceneComponent> ForeL;
	UPROPERTY() TObjectPtr<USceneComponent> ForeR;
	UPROPERTY() TObjectPtr<USceneComponent> HandL;
	UPROPERTY() TObjectPtr<USceneComponent> HandR;
	UPROPERTY() TObjectPtr<USceneComponent> WeaponPivot;
	UPROPERTY() TObjectPtr<USceneComponent> LegL;
	UPROPERTY() TObjectPtr<USceneComponent> LegR;
	UPROPERTY() TObjectPtr<USceneComponent> ShinL;
	UPROPERTY() TObjectPtr<USceneComponent> ShinR;

	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Meshes;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> FlashMat;

	TWeakObjectPtr<AMusicDirector> Music;

	FName Action = NAME_None;
	float ActionT = 0.f;
	float ActionLen = 1.f;
	bool bActionHold = false;
	float Time = 0.f;
	float RunPhase = 0.f;
	float FlashLeft = 0.f;

	/** Current joint rotations (radians, Godot-style axes) for smoothing. */
	TMap<FName, FVector> Current;
	FVector HipsOffset = FVector::ZeroVector;
	FVector DownRot = FVector::ZeroVector;
	FVector DownPos = FVector::ZeroVector;
};

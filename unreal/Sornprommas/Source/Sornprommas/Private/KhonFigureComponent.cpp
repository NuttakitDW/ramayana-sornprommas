#include "KhonFigureComponent.h"

#include "Components/StaticMeshComponent.h"
#include "KhonCrowns.h"
#include "KhonParts.h"
#include "KhonPoses.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MusicDirector.h"
#include "SornGameMode.h"
#include "SornTypes.h"

using namespace KhonParts;

namespace
{
	constexpr float HipY = 0.98f;
	constexpr float PoseSharpness = 14.f;
}

UKhonFigureComponent::UKhonFigureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKhonFigureComponent::Build(const FKhonSpec& InSpec)
{
	Spec = InSpec;
	AActor* O = GetOwner();
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* GoldM = Gold(this);
	UMaterialInterface* SkinM = L->Mat(Spec.Skin, 0.f, 0.55f);
	UMaterialInterface* ClothM = L->Mat(Spec.Cloth, 0.1f, 0.5f);
	UMaterialInterface* TrimM = L->Mat(Spec.Trim, 0.1f, 0.5f);

	RootPivot = Pivot(O, this, FVector::ZeroVector, TEXT("FigRoot"));
	Hips = Pivot(O, RootPivot, FVector(0, HipY, 0));

	// Lower costume: ผ้านุ่ง, belt, ห้อยหน้า (front panel), หางหงส์ (rear panel).
	Part(O, Hips, EKhonShape::Cube, ClothM, FVector(0, 0.02, 0), FVector(0.36, 0.2, 0.24));
	Part(O, Hips, EKhonShape::Cylinder, ClothM, FVector(0, -0.12, 0), FVector(0.5, 0.32, 0.5));
	Part(O, Hips, EKhonShape::Cylinder, GoldM, FVector(0, 0.1, 0), FVector(0.46, 0.06, 0.46));
	Part(O, Hips, EKhonShape::Cube, TrimM, FVector(0, -0.2, -0.26), FVector(0.15, 0.46, 0.02));
	Part(O, Hips, EKhonShape::Cube, GoldM, FVector(0, -0.2, -0.25), FVector(0.17, 0.48, 0.015));
	Part(O, Hips, EKhonShape::Cube, TrimM, FVector(0, -0.16, 0.27), FVector(0.26, 0.5, 0.02), FVector(-14, 0, 0));

	// Chest: suit, กรองคอ collar, สังวาล sashes, ทับทรวง pendant, อินทรธนู epaulettes.
	Chest = Pivot(O, Hips, FVector(0, 0.12, 0));
	Part(O, Chest, EKhonShape::Sphere, SkinM, FVector(0, 0.26, 0), FVector(0.45, 0.66, 0.34));
	Part(O, Chest, EKhonShape::Cylinder, GoldM, FVector(0, 0.52, 0), FVector(0.52, 0.05, 0.52));
	Part(O, Chest, EKhonShape::Cylinder, TrimM, FVector(0, 0.53, 0), FVector(0.42, 0.055, 0.42));
	Part(O, Chest, EKhonShape::Cube, GoldM, FVector(0.02, 0.26, -0.165), FVector(0.05, 0.62, 0.02), FVector(0, 0, 32));
	Part(O, Chest, EKhonShape::Cube, GoldM, FVector(-0.02, 0.26, -0.165), FVector(0.05, 0.62, 0.02), FVector(0, 0, -32));
	Part(O, Chest, EKhonShape::Cube, GoldM, FVector(0, 0.34, -0.18), FVector(0.09, 0.09, 0.03), FVector(0, 0, 45));
	for (const float Side : {-1.f, 1.f})
	{
		Part(O, Chest, EKhonShape::Cone, GoldM, FVector(0.29f * Side, 0.56f, 0), FVector(0.16, 0.22, 0.12), FVector(0, 0, -28.f * Side));
	}

	Head = Pivot(O, Chest, FVector(0, 0.62, 0));
	KhonCrowns::BuildHead(O, Head, Spec);

	BuildArm(-1.f, SkinM, GoldM, ArmL, ForeL, HandL);
	BuildArm(1.f, SkinM, GoldM, ArmR, ForeR, HandR);
	WeaponPivot = Pivot(O, HandR, FVector::ZeroVector);
	BuildWeapon();

	BuildLeg(-1.f, ClothM, GoldM, SkinM, LegL, ShinL);
	BuildLeg(1.f, ClothM, GoldM, SkinM, LegR, ShinR);
	if (Spec.bTail)
	{
		BuildTail(SkinM);
	}
	if (Spec.Kind == EKhonKind::Yak)
	{
		Hips->SetRelativeScale3D(FVector(1.1f, 1.12f, 1.f));
	}

	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);
	for (USceneComponent* C : Children)
	{
		if (UStaticMeshComponent* M = Cast<UStaticMeshComponent>(C))
		{
			Meshes.Add(M);
		}
	}
	FlashMat = L->UniqueMat(this, FLinearColor::White, 4.f, true);
}

void UKhonFigureComponent::BuildArm(float Side, UMaterialInterface* SkinM, UMaterialInterface* GoldM,
	TObjectPtr<USceneComponent>& OutArm, TObjectPtr<USceneComponent>& OutFore, TObjectPtr<USceneComponent>& OutHand)
{
	AActor* O = GetOwner();
	OutArm = Pivot(O, Chest, FVector(0.28f * Side, 0.46f, 0));
	Part(O, OutArm, EKhonShape::Sphere, SkinM, FVector(0, -0.16, 0), FVector(0.14, 0.38, 0.14));
	OutFore = Pivot(O, OutArm, FVector(0, -0.31, 0));
	Part(O, OutFore, EKhonShape::Sphere, SkinM, FVector(0, -0.15, 0), FVector(0.12, 0.34, 0.12));
	Part(O, OutFore, EKhonShape::Cylinder, GoldM, FVector(0, -0.24, 0), FVector(0.15, 0.05, 0.15));
	OutHand = Pivot(O, OutFore, FVector(0, -0.3, 0));
	Part(O, OutHand, EKhonShape::Sphere, SkinM, FVector::ZeroVector, FVector(0.12, 0.12, 0.12));
}

void UKhonFigureComponent::BuildLeg(float Side, UMaterialInterface* ClothM, UMaterialInterface* GoldM, UMaterialInterface* SkinM,
	TObjectPtr<USceneComponent>& OutLeg, TObjectPtr<USceneComponent>& OutShin)
{
	AActor* O = GetOwner();
	OutLeg = Pivot(O, Hips, FVector(0.12f * Side, -0.06f, 0));
	Part(O, OutLeg, EKhonShape::Sphere, ClothM, FVector(0, -0.21, 0), FVector(0.18, 0.48, 0.18));
	OutShin = Pivot(O, OutLeg, FVector(0, -0.44, 0));
	Part(O, OutShin, EKhonShape::Cylinder, GoldM, FVector(0, -0.02, 0), FVector(0.2, 0.05, 0.2));
	Part(O, OutShin, EKhonShape::Sphere, SkinM, FVector(0, -0.21, 0), FVector(0.14, 0.46, 0.14));
	USceneComponent* Foot = Pivot(O, OutShin, FVector(0, -0.43, 0));
	Part(O, Foot, EKhonShape::Cube, SkinM, FVector(0, -0.02, -0.05), FVector(0.1, 0.06, 0.22));
}

void UKhonFigureComponent::BuildTail(UMaterialInterface* SkinM)
{
	AActor* O = GetOwner();
	USceneComponent* Parent = Pivot(O, Hips, FVector(0, -0.05, 0.22));
	for (int32 I = 0; I < 7; ++I)
	{
		USceneComponent* Seg = Pivot(O, Parent, FVector(0, I == 0 ? 0.f : 0.12f, 0));
		const float R = 0.07f - I * 0.006f;
		Part(O, Seg, EKhonShape::Sphere, SkinM, FVector(0, 0.06, 0), FVector(R, 0.17, R));
		TailSegments.Add(Seg);
		Parent = Seg;
	}
	TailSegments[0]->SetRelativeRotation(Sorn::R(FVector(1.9f, 0, 0)));
}

void UKhonFigureComponent::BuildWeapon()
{
	AActor* O = GetOwner();
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* GoldM = Gold(this);
	UMaterialInterface* Shaft = L->Mat(Sorn::Col::KhonRedDark, 0.3f, 0.4f);
	switch (Spec.Weapon)
	{
	case EKhonWeapon::Trident:
		// ตรีเพชร — prongs along +Y of the weapon pivot.
		Part(O, WeaponPivot, EKhonShape::Cylinder, Shaft, FVector(0, 0.35, 0), FVector(0.048, 1.7, 0.048));
		Part(O, WeaponPivot, EKhonShape::Sphere, GoldM, FVector(0, 1.2, 0), FVector(0.1, 0.1, 0.1));
		Part(O, WeaponPivot, EKhonShape::Cone, GoldM, FVector(0, 1.43, 0), FVector(0.09, 0.42, 0.09));
		for (const float Side : {-1.f, 1.f})
		{
			Part(O, WeaponPivot, EKhonShape::Cylinder, GoldM, FVector(0.07f * Side, 1.23f, 0), FVector(0.036, 0.16, 0.036), FVector(0, 0, -60.f * Side));
			Part(O, WeaponPivot, EKhonShape::Cone, GoldM, FVector(0.13f * Side, 1.36f, 0), FVector(0.07, 0.3, 0.07));
		}
		break;
	case EKhonWeapon::Club:
		// กระบอง
		Part(O, WeaponPivot, EKhonShape::Cylinder, Shaft, FVector(0, 0.38, 0), FVector(0.12, 1.0, 0.12));
		for (const float Y : {0.12f, 0.5f, 0.84f})
		{
			Part(O, WeaponPivot, EKhonShape::Cylinder, GoldM, FVector(0, Y, 0), FVector(0.14, 0.035, 0.14));
		}
		break;
	case EKhonWeapon::Bow:
	{
		// ธนู held in the left hand: a gilded arc of short segments.
		USceneComponent* Bow = Pivot(O, HandL, FVector::ZeroVector);
		for (int32 I = 0; I < 7; ++I)
		{
			const float A = FMath::DegreesToRadians(-60.f + I * 20.f);
			Part(O, Bow, EKhonShape::Cylinder, GoldM, FVector(0, FMath::Sin(A) * 0.55f, -FMath::Cos(A) * 0.18f + 0.1f),
				FVector(0.04, 0.2, 0.04), FVector(FMath::RadiansToDegrees(A * 0.35f), 0, 0));
		}
		Bow->SetRelativeRotation(Sorn::R(FVector(-UE_PI / 2.f, 0, 0)));
		break;
	}
	default:
		break;
	}
}

void UKhonFigureComponent::Play(FName InAction, float Duration, bool bHold)
{
	Action = InAction;
	ActionT = 0.f;
	ActionLen = FMath::Max(Duration, 0.01f);
	bActionHold = bHold;
}

void UKhonFigureComponent::Flash(const FLinearColor& Color, float Duration)
{
	UKhonMaterialSubsystem::SetMatColor(FlashMat, Color, 0.75f);
	FlashLeft = Duration;
	for (UStaticMeshComponent* M : Meshes)
	{
		M->SetOverlayMaterial(FlashMat);
	}
}

float UKhonFigureComponent::BeatPhase()
{
	if (!Music.IsValid())
	{
		if (const ASornGameMode* GM = ASornGameMode::Get(this))
		{
			Music = GM->Music.Get();
		}
	}
	return Music.IsValid() ? Music->BeatPhase : FMath::Fmod(Time * 1.2f, 1.f);
}

void UKhonFigureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!Hips)
	{
		return;
	}
	Time += DeltaTime;
	if (!Action.IsNone())
	{
		ActionT += DeltaTime;
		if (ActionT >= ActionLen && !bActionHold)
		{
			Action = NAME_None;
		}
	}
	if (FlashLeft > 0.f)
	{
		FlashLeft -= DeltaTime;
		if (FlashLeft <= 0.f)
		{
			for (UStaticMeshComponent* M : Meshes)
			{
				M->SetOverlayMaterial(nullptr);
			}
		}
	}
	if (MoveRatio > 0.05f && bGrounded)
	{
		RunPhase += DeltaTime * (7.f + 6.f * MoveRatio);
	}
	ApplyPose(DeltaTime);
}

void UKhonFigureComponent::ApplyPose(float DeltaTime)
{
	FKhonPoseContext Ctx;
	Ctx.Kind = Spec.Kind;
	Ctx.Time = Time;
	Ctx.Beat = BeatPhase();
	Ctx.Move = MoveRatio;
	Ctx.bGrounded = bGrounded;
	Ctx.Action = Action;
	Ctx.U = Action.IsNone() ? 0.f : FMath::Clamp(ActionT / ActionLen, 0.f, 1.f);
	Ctx.RunPhase = RunPhase;
	const FKhonPose Pose = KhonPoses::Compute(Ctx);

	const float K = 1.f - FMath::Exp(-PoseSharpness * DeltaTime);
	const float Snap = 1.f - FMath::Exp(-40.f * DeltaTime);
	const float W = Action.IsNone() ? K : Snap;

	const TPair<const TCHAR*, USceneComponent*> Joints[] = {
		{TEXT("Chest"), Chest}, {TEXT("Head"), Head}, {TEXT("ArmL"), ArmL}, {TEXT("ArmR"), ArmR},
		{TEXT("ForeL"), ForeL}, {TEXT("ForeR"), ForeR}, {TEXT("LegL"), LegL}, {TEXT("LegR"), LegR},
		{TEXT("ShinL"), ShinL}, {TEXT("ShinR"), ShinR}, {TEXT("Weapon"), WeaponPivot},
	};
	for (const TPair<const TCHAR*, USceneComponent*>& J : Joints)
	{
		const FName Key(J.Key);
		FVector& Cur = Current.FindOrAdd(Key);
		Cur = FMath::Lerp(Cur, Pose.Get(J.Key), W);
		J.Value->SetRelativeRotation(Sorn::R(Cur));
	}
	HipsOffset = FMath::Lerp(HipsOffset, Pose.HipsOffset, K);
	Hips->SetRelativeLocation(Sorn::G(HipsOffset.X, HipY + HipsOffset.Y, HipsOffset.Z));

	const float DK = 1.f - FMath::Exp(-6.f * DeltaTime);
	DownRot = FMath::Lerp(DownRot, bDown ? FVector(-1.45f, 0, 0) : FVector::ZeroVector, DK);
	DownPos = FMath::Lerp(DownPos, bDown ? FVector(0, 0.25f, 0.4f) : FVector::ZeroVector, DK);
	RootPivot->SetRelativeRotation(Sorn::R(DownRot));
	RootPivot->SetRelativeLocation(Sorn::G(DownPos.X, DownPos.Y, DownPos.Z));

	for (int32 I = 1; I < TailSegments.Num(); ++I)
	{
		TailSegments[I]->SetRelativeRotation(Sorn::R(FVector(
			-0.32f + FMath::Sin(Time * 3.f + I * 0.6f) * 0.08f, 0.f, FMath::Sin(Time * 2.2f + I * 0.5f) * 0.12f)));
	}
}

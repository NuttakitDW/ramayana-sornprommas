#include "ErawanBoss.h"

#include "Engine/World.h"
#include "HanumanCharacter.h"
#include "KhonFigureComponent.h"
#include "KhonParts.h"
#include "MusicDirector.h"
#include "SornCombat.h"
#include "SornFx.h"
#include "SornGameMode.h"
#include "SornTypes.h"
#include "TimerManager.h"

using namespace KhonParts;

namespace
{
	constexpr float HoverHeight = 550.f;
	constexpr float OrbitRadius = 1100.f;
	constexpr float ChargeSpeed = 2100.f;
	constexpr float KneelTime = 4.2f;
	constexpr int32 TrunkLen = 6;
}

AErawanBoss::AErawanBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AErawanBoss::BeginPlay()
{
	Super::BeginPlay();
	BuildModel();
	if (ASornGameMode* GM = ASornGameMode::Get(this))
	{
		GM->RegisterHittable(this);
	}
}

void AErawanBoss::EndPlay(const EEndPlayReason::Type Reason)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this))
	{
		GM->UnregisterHittable(this);
	}
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(Reason);
}

// ------------------------------------------------------------------ model

void AErawanBoss::BuildModel()
{
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* White = L->Mat(Sorn::Col::ErawanWhite, 0.f, 0.5f);
	UMaterialInterface* GoldM = Gold(this);
	UMaterialInterface* Red = L->Mat(Sorn::Col::KhonRed, 0.1f, 0.5f);

	Body = Pivot(this, RootComponent, FVector(0, 2.2f, 0));
	Part(this, Body, EKhonShape::Sphere, White, FVector::ZeroVector, FVector(2.6, 2.6, 4.9));
	// Caparison (ผ้าปูหลัง) with gold border.
	Part(this, Body, EKhonShape::Cube, Red, FVector(0, 1.28f, 0.1f), FVector(2.75, 0.1, 2.7));
	Part(this, Body, EKhonShape::Cube, GoldM, FVector(0, 1.23f, 0.1f), FVector(2.85, 0.06, 2.8));
	for (const float Side : {-1.f, 1.f})
	{
		Part(this, Body, EKhonShape::Cube, Red, FVector(1.36f * Side, 0.65f, 0.1f), FVector(0.06, 1.2, 2.5));
		Part(this, Body, EKhonShape::Cube, GoldM, FVector(1.37f * Side, 0.06f, 0.1f), FVector(0.07, 0.12, 2.6));
	}
	for (const float X : {-0.75f, 0.75f})
	{
		for (const float Z : {-1.35f, 1.35f})
		{
			USceneComponent* Leg = Pivot(this, Body, FVector(X, -0.6f, Z));
			Part(this, Leg, EKhonShape::Cylinder, White, FVector(0, -0.9f, 0), FVector(0.8, 2.0, 0.8));
			Part(this, Leg, EKhonShape::Cylinder, GoldM, FVector(0, -1.7f, 0), FVector(0.9, 0.12, 0.9));
			Legs.Add(Leg);
		}
	}
	Part(this, Body, EKhonShape::Cylinder, White, FVector(0, 0.2f, 2.55f), FVector(0.12, 1.2, 0.12), FVector(-160, 0, 0));
	for (int32 I = 0; I < 3; ++I)
	{
		Heads.Add(BuildHead(I, White, GoldM));
	}
	BuildPavilion(GoldM, Red);
}

USceneComponent* AErawanBoss::BuildHead(int32 I, UMaterialInterface* White, UMaterialInterface* GoldM)
{
	UKhonMaterialSubsystem* L = Lib(this);
	USceneComponent* Head = Pivot(this, Body, FVector((I - 1) * 1.05f, 0.55f, -2.35f));
	Head->SetRelativeRotation(Sorn::R(FVector(0, -(I - 1) * 0.3f, 0)));
	Part(this, Head, EKhonShape::Sphere, White, FVector::ZeroVector, FVector(1.44, 1.5, 1.58));
	for (const float Side : {-1.f, 1.f})
	{
		Part(this, Head, EKhonShape::Cylinder, White, FVector(0.62f * Side, 0.05f, 0.2f), FVector(1.1, 0.06, 1.3), FVector(0, 0, 90));
		Part(this, Head, EKhonShape::Sphere, L->Mat(Sorn::Col::Pupil), FVector(0.32f * Side, 0.12f, -0.6f), FVector(0.12));
		Part(this, Head, EKhonShape::Cone, L->Mat(Sorn::Col::Fang, 0.1f, 0.25f), FVector(0.24f * Side, -0.4f, -0.85f), FVector(0.14, 0.8, 0.14), FVector(-65, 0, 0));
	}
	// เทริด crown.
	Part(this, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.62f, -0.05f), FVector(0.8, 0.18, 0.8));
	Part(this, Head, EKhonShape::Cone, GoldM, FVector(0, 0.98f, -0.05f), FVector(0.6, 0.55, 0.6));
	Part(this, Head, EKhonShape::Cube, GoldM, FVector(0, 0.25f, -0.73f), FVector(0.5, 0.35, 0.05));
	// Trunk (งวง).
	USceneComponent* Parent = Pivot(this, Head, FVector(0, -0.25f, -0.72f));
	for (int32 S = 0; S < TrunkLen; ++S)
	{
		USceneComponent* Seg = Pivot(this, Parent, FVector(0, S == 0 ? 0.f : -0.28f, 0));
		const float D = 2.f * (0.19f - S * 0.022f);
		Part(this, Seg, EKhonShape::Sphere, White, FVector(0, -0.14f, 0), FVector(D, 0.36, D));
		TrunkSegments.Add(Seg);
		Parent = Seg;
	}
	return Head;
}

void AErawanBoss::BuildPavilion(UMaterialInterface* GoldM, UMaterialInterface* Red)
{
	USceneComponent* Base = Pivot(this, Body, FVector(0, 1.35f, 0.2f));
	Part(this, Base, EKhonShape::Cube, GoldM, FVector::ZeroVector, FVector(1.7, 0.18, 1.7));
	for (const float X : {-0.72f, 0.72f})
	{
		for (const float Z : {-0.72f, 0.72f})
		{
			Part(this, Base, EKhonShape::Cylinder, GoldM, FVector(X, 1.05f, Z), FVector(0.1, 2.1, 0.1));
		}
	}
	for (int32 I = 0; I < 4; ++I)
	{
		const float W = 1.9f - I * 0.38f;
		Part(this, Base, EKhonShape::Cube, I % 2 == 1 ? Red : GoldM, FVector(0, 2.1f + I * 0.2f, 0), FVector(W, 0.16, W));
	}
	Part(this, Base, EKhonShape::Cone, GoldM, FVector(0, 3.55f, 0), FVector(0.56, 1.4, 0.56));

	RiderFigure = NewObject<UKhonFigureComponent>(this);
	RiderFigure->SetupAttachment(Base);
	RiderFigure->RegisterComponent();
	RiderFigure->SetRelativeLocation(Sorn::G(0, 0.1f, -0.1f));
	FKhonSpec Spec;
	Spec.Kind = EKhonKind::Phra;
	Spec.Skin = Sorn::Col::IndraGreen;
	Spec.Cloth = Sorn::Col::GoldDeep;
	Spec.Trim = Sorn::Col::KhonRed;
	Spec.Crown = EKhonCrown::Phra;
	Spec.Weapon = EKhonWeapon::Bow;
	Spec.Mouth = EKhonMouth::Calm;
	RiderFigure->Build(Spec);
}

void AErawanBoss::SetKneel(float Amount)
{
	Body->SetRelativeLocation(Sorn::G(0, FMath::Lerp(2.2f, 1.35f, Amount), 0));
	Body->SetRelativeRotation(Sorn::R(FVector(FMath::Lerp(0.f, -0.14f, Amount), 0, 0)));
	for (USceneComponent* Leg : Legs)
	{
		const bool bFront = Leg->GetRelativeLocation().X > 0.f;
		Leg->SetRelativeRotation(Sorn::R(FVector(FMath::Lerp(0.f, bFront ? -1.2f : 1.2f, Amount) * 0.6f, 0, 0)));
	}
}

void AErawanBoss::AnimateTrunks(float Dt)
{
	for (int32 I = 0; I < TrunkSegments.Num(); ++I)
	{
		const int32 HeadI = I / TrunkLen;
		const int32 S = I % TrunkLen;
		TrunkSegments[I]->SetRelativeRotation(Sorn::R(FVector(
			(S > 0 ? 0.18f : 0.35f) + FMath::Sin(Time * 1.8f + HeadI + S * 0.4f) * 0.12f, 0,
			FMath::Sin(Time * 1.3f + HeadI * 2.f + S * 0.3f) * 0.08f)));
	}
	for (int32 I = 0; I < 3; ++I)
	{
		if (HeadBreakT[I] >= 0.f)
		{
			HeadBreakT[I] += Dt;
			const float U = FMath::Clamp(HeadBreakT[I] / 0.6f, 0.f, 1.f);
			Heads[I]->SetRelativeRotation(Sorn::R(FVector(1.2f * U, -(I - 1) * 0.3f, 0)));
			Heads[I]->SetRelativeScale3D(FVector(FMath::Max(0.02f, 1.f - U)));
			if (U >= 1.f)
			{
				Heads[I]->SetVisibility(false, true);
			}
		}
	}
}

// ------------------------------------------------------------------ hittable

int32 AErawanBoss::HeadsLeft() const
{
	int32 N = 0;
	for (const float V : HeadHp)
	{
		N += V > 0.f ? 1 : 0;
	}
	return N;
}

TArray<FVector> AErawanBoss::HurtPoints() const
{
	TArray<FVector> Points;
	for (int32 I = 0; I < 3; ++I)
	{
		Points.Add(HeadHp[I] > 0.f && Heads.IsValidIndex(I) ? Heads[I]->GetComponentLocation() : FVector(0, 0, -100000.f));
	}
	return Points;
}

bool AErawanBoss::ReceiveHit(float Damage, const FVector& /*From*/, float /*Knock*/, int32 Index)
{
	if (!IsAlive() || Index < 0 || Index > 2 || HeadHp[Index] <= 0.f)
	{
		return false;
	}
	HeadHp[Index] -= Damage;
	RiderFigure->Flash(FLinearColor::White, 0.08f);
	if (HeadHp[Index] <= 0.f)
	{
		HeadHp[Index] = 0.f;
		BreakHead(Index);
	}
	return true;
}

void AErawanBoss::BreakHead(int32 I)
{
	HeadBreakT[I] = 0.f;
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Fx)
	{
		const FVector Pos = Heads[I]->GetComponentLocation();
		GM->Fx->Sparks(Pos, Sorn::Col::Gold, 40, 1000.f);
		GM->Fx->Ring(FVector(Pos.X, Pos.Y, 0.f), 500.f, Sorn::Col::UiSignal, 0.5f);
		GM->Fx->HitStop(0.18f);
	}
	Sfx(TEXT("klong"), 0.45f, 1.4f);
	const int32 Left = HeadsLeft();
	OnHeadBroken.Broadcast(I, Left);
	if (Left == 0)
	{
		State = TEXT("dying");
		OnDefeated.Broadcast();
	}
	else if (State == TEXT("kneel"))
	{
		T = FMath::Min(T, 0.6f);
	}
}

// ------------------------------------------------------------------ behaviour

void AErawanBoss::Appear()
{
	SetActorLocation(Sorn::G(0, 24, -40));
	State = TEXT("enter");
	T = 3.2f;
}

FVector AErawanBoss::OrbitPoint() const
{
	return FVector(-FMath::Sin(OrbitA) * OrbitRadius, FMath::Cos(OrbitA) * OrbitRadius, HoverHeight);
}

void AErawanBoss::FacePlayer(float Dt, float Rate)
{
	const ASornGameMode* GM = ASornGameMode::Get(this);
	const AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr;
	if (!H)
	{
		return;
	}
	const float Target = SornCombat::YawToward(SornCombat::Flat(H->GetActorLocation() - GetActorLocation()));
	SetActorRotation(FRotator(0, FMath::FixedTurn(GetActorRotation().Yaw, Target, FMath::RadiansToDegrees(Rate) * Dt), 0));
}

void AErawanBoss::GoHover()
{
	State = TEXT("hover");
	T = FMath::FRandRange(2.2f, 3.2f);
	if (bActive)
	{
		Cue(TEXT("choet_klong"));
	}
}

void AErawanBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Dt = DeltaSeconds;
	Time += Dt;
	T -= Dt;
	const ASornGameMode* GM = ASornGameMode::Get(this);
	AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr;
	const FVector Loc = GetActorLocation();

	if (State == TEXT("enter"))
	{
		SetActorLocation(FMath::Lerp(Loc, OrbitPoint(), 1.f - FMath::Exp(-1.6f * Dt)));
		FacePlayer(Dt);
		if (T <= 0.f) GoHover();
	}
	else if (State == TEXT("hover"))
	{
		OrbitA += Dt * 0.35f;
		const FVector Goal = OrbitPoint() + FVector(0, 0, FMath::Sin(Time * 2.f) * 40.f);
		SetActorLocation(FMath::Lerp(Loc, Goal, 1.f - FMath::Exp(-2.5f * Dt)));
		FacePlayer(Dt);
		if (T <= 0.f)
		{
			if (!bActive || !H || !H->IsAliveHanuman())
			{
				T = 1.f;
			}
			else if (VolleysInRow >= 1 || FMath::FRand() < 0.35f)
			{
				StartCharge();
			}
			else
			{
				StartVolley();
			}
		}
	}
	else if (State == TEXT("volley"))
	{
		FacePlayer(Dt);
		if (T < 1.4f && RiderFigure->GetAction() == TEXT("bow_draw"))
		{
			RiderFigure->Play(TEXT("bow_release"), 0.5f);
		}
		if (T <= 0.f) GoHover();
	}
	else if (State == TEXT("charge_windup"))
	{
		FacePlayer(Dt, 8.f);
		SetActorLocation(FVector(Loc.X, Loc.Y, FMath::Lerp(Loc.Z, 300.f, 1.f - FMath::Exp(-3.f * Dt))));
		if (T <= 0.f && H)
		{
			const FVector Target(H->GetActorLocation().X, H->GetActorLocation().Y, 0.f);
			const FVector Dir = SornCombat::Flat(Target - Loc).GetSafeNormal();
			ChargeTo = Target + Dir * 500.f;
			FVector2D Flat2(ChargeTo.X, ChargeTo.Y);
			if (Flat2.Size() > 1600.f)
			{
				Flat2 = Flat2.GetSafeNormal() * 1600.f;
				ChargeTo = FVector(Flat2.X, Flat2.Y, 0.f);
			}
			bChargeHit = false;
			State = TEXT("charge");
			SetActorRotation(FRotator(0, SornCombat::YawToward(Dir), 0));
		}
	}
	else if (State == TEXT("charge"))
	{
		const FVector To = ChargeTo - Loc;
		const float Step = ChargeSpeed * Dt;
		if (To.Size() <= Step)
		{
			SetActorLocation(ChargeTo);
			Land();
		}
		else
		{
			SetActorLocation(Loc + To.GetSafeNormal() * Step);
			if (!bChargeHit && H)
			{
				const float D = SornCombat::Flat(H->GetActorLocation() - Loc).Size();
				if (D < 260.f && Loc.Z < 350.f)
				{
					bChargeHit = H->TakeHit(22.f, Loc, 1200.f);
				}
			}
		}
	}
	else if (State == TEXT("kneel"))
	{
		Kneel = FMath::FInterpConstantTo(Kneel, 1.f, Dt, 3.f);
		if (T <= 0.f)
		{
			State = TEXT("rise");
			T = 1.2f;
		}
	}
	else if (State == TEXT("rise"))
	{
		Kneel = FMath::FInterpConstantTo(Kneel, 0.f, Dt, 2.f);
		SetActorLocation(FMath::Lerp(Loc, OrbitPoint(), 1.f - FMath::Exp(-1.8f * Dt)));
		if (T <= 0.f) GoHover();
	}
	SetKneel(Kneel);
	AnimateTrunks(Dt);
}

void AErawanBoss::StartVolley()
{
	State = TEXT("volley");
	T = 2.4f;
	++VolleysInRow;
	RiderFigure->Play(TEXT("bow_draw"), 0.9f, true);
	Cue(TEXT("choet_ching"));
	const AHanumanCharacter* H = ASornGameMode::Get(this)->Hanuman();
	const int32 Count = 5 + (3 - HeadsLeft()) * 2;
	for (int32 I = 0; I < Count; ++I)
	{
		FVector Offset = FVector::ZeroVector;
		if (I > 0)
		{
			Offset = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal() * FMath::FRandRange(150.f, 500.f);
		}
		const FVector Lead = I == 0 ? H->GetVelocity() * 0.5f : FVector::ZeroVector;
		ScheduleArrow(H->GetActorLocation() + SornCombat::Flat(Lead) + Offset, 1.f + I * 0.08f);
	}
}

void AErawanBoss::ScheduleArrow(const FVector& At, float Delay)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Fx)
	{
		GM->Fx->Telegraph(At, 150.f, Delay, Sorn::Col::Danger);
	}
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &AErawanBoss::ArrowLand, At), Delay, false);
}

void AErawanBoss::ArrowLand(FVector At)
{
	if (State == TEXT("dying") || State == TEXT("dead"))
	{
		return;
	}
	ASornGameMode* GM = ASornGameMode::Get(this);
	if (GM && GM->Fx)
	{
		GM->Fx->Sparks(FVector(At.X, At.Y, 30.f), Sorn::Col::Gold, 12, 500.f);
		GM->Fx->Ring(FVector(At.X, At.Y, 0.f), 170.f, Sorn::Col::Danger, 0.3f);
	}
	Sfx(TEXT("taphon_hi"), 1.6f, 0.4f);
	AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr;
	if (H && SornCombat::Flat(H->GetActorLocation() - At).Size() < 160.f && H->GetActorLocation().Z < 240.f)
	{
		H->TakeHit(13.f, At, 500.f);
	}
}

void AErawanBoss::StartCharge()
{
	VolleysInRow = 0;
	State = TEXT("charge_windup");
	T = 1.f;
	Sfx(TEXT("klong"), 0.6f, 1.4f);
}

void AErawanBoss::Land()
{
	State = TEXT("kneel");
	T = KneelTime;
	ASornGameMode* GM = ASornGameMode::Get(this);
	if (GM && GM->Fx)
	{
		GM->Fx->Ring(GetActorLocation(), 600.f, Sorn::Col::Gold, 0.5f);
	}
	if (AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr)
	{
		H->Shake(0.5f);
	}
	Sfx(TEXT("klong"), 0.5f, 1.6f);
}

void AErawanBoss::Cue(const FString& Id)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
	{
		GM->Music->PlayCue(Id);
	}
}

void AErawanBoss::Sfx(const FString& Kind, float Pitch, float Volume)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
	{
		GM->Music->Sfx(Kind, Pitch, Volume);
	}
}

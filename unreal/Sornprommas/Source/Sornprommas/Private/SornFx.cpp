#include "SornFx.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "KhonParts.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SornTypes.h"

using namespace KhonParts;

ASornFx::ASornFx()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ASornFx::HitStop(float Seconds)
{
	HitStopUntil = FMath::Max(HitStopUntil, FPlatformTime::Seconds() + Seconds);
}

void ASornFx::Sparks(const FVector& Pos, const FLinearColor& Color, int32 Amount, float Speed)
{
	UMaterialInterface* M = Lib(this)->Mat(Color, 0.f, 0.4f, 3.f);
	for (int32 I = 0; I < Amount; ++I)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetStaticMesh(Lib(this)->Mesh(EKhonShape::Sphere));
		C->SetMaterial(0, M);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetupAttachment(RootComponent);
		C->SetUsingAbsoluteLocation(true);
		C->SetUsingAbsoluteScale(true);
		C->RegisterComponent();
		C->SetWorldLocation(Pos);
		C->SetWorldScale3D(FVector(FMath::FRandRange(0.04f, 0.1f)));
		const FVector Dir = FMath::VRand();
		SparkList.Add({C, Dir * FMath::FRandRange(Speed * 0.4f, Speed) + FVector(0, 0, Speed * 0.3f), 0.5f});
	}
}

USceneComponent* ASornFx::MakeRing(const FVector& Pos, float Radius, UMaterialInstanceDynamic* Mat)
{
	USceneComponent* Root = NewObject<USceneComponent>(this);
	Root->SetupAttachment(RootComponent);
	Root->SetUsingAbsoluteLocation(true);
	Root->SetUsingAbsoluteScale(true);
	Root->RegisterComponent();
	Root->SetWorldLocation(Pos + FVector(0, 0, 6));
	constexpr int32 Segments = 32;
	for (int32 I = 0; I < Segments; ++I)
	{
		const float A = UE_TWO_PI * I / Segments;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetStaticMesh(Lib(this)->Mesh(EKhonShape::Cube));
		C->SetMaterial(0, Mat);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetupAttachment(Root);
		C->SetRelativeLocation(FVector(FMath::Cos(A), FMath::Sin(A), 0.f) * 100.f);
		C->SetRelativeRotation(FRotator(0.f, FMath::RadiansToDegrees(A), 0.f));
		C->SetRelativeScale3D(FVector(0.12f, 0.22f, 0.04f));
		C->RegisterComponent();
	}
	Root->SetWorldScale3D(FVector(Radius / 100.f, Radius / 100.f, 1.f));
	return Root;
}

void ASornFx::Ring(const FVector& Pos, float Radius, const FLinearColor& Color, float Duration)
{
	UMaterialInstanceDynamic* M = Lib(this)->UniqueMat(this, Color, 2.f, true);
	USceneComponent* R = MakeRing(Pos, 20.f, M);
	FTimed T;
	T.Kind = ETimedKind::Ring;
	T.Comp = R;
	T.Mat = M;
	T.Color = Color;
	T.Duration = Duration;
	T.StartScale = 0.2f;
	T.EndScale = Radius / 100.f;
	TimedList.Add(T);
}

void ASornFx::Telegraph(const FVector& Pos, float Radius, float Duration, const FLinearColor& Color)
{
	UMaterialInstanceDynamic* EdgeM = Lib(this)->UniqueMat(this, Color, 2.f, true);
	UKhonMaterialSubsystem::SetMatColor(EdgeM, Color, 0.9f);
	USceneComponent* Edge = MakeRing(FVector(Pos.X, Pos.Y, 0.f), Radius, EdgeM);
	FTimed EdgeT;
	EdgeT.Kind = ETimedKind::Ring;
	EdgeT.Comp = Edge;
	EdgeT.Duration = Duration;
	EdgeT.StartScale = EdgeT.EndScale = Radius / 100.f;
	EdgeT.bFade = false;
	TimedList.Add(EdgeT);

	UMaterialInstanceDynamic* FillM = Lib(this)->UniqueMat(this, Color, 1.f, true);
	UKhonMaterialSubsystem::SetMatColor(FillM, Color, 0.35f);
	UStaticMeshComponent* Disc = NewObject<UStaticMeshComponent>(this);
	Disc->SetStaticMesh(Lib(this)->Mesh(EKhonShape::Cylinder));
	Disc->SetMaterial(0, FillM);
	Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Disc->SetCastShadow(false);
	Disc->SetupAttachment(RootComponent);
	Disc->SetUsingAbsoluteLocation(true);
	Disc->SetUsingAbsoluteScale(true);
	Disc->RegisterComponent();
	Disc->SetWorldLocation(FVector(Pos.X, Pos.Y, 4.f));
	FTimed DiscT;
	DiscT.Kind = ETimedKind::Disc;
	DiscT.Comp = Disc;
	DiscT.Duration = Duration;
	DiscT.StartScale = 0.05f;
	DiscT.EndScale = Radius / 50.f;
	DiscT.bFade = false;
	TimedList.Add(DiscT);
}

void ASornFx::Number(const FVector& Pos, float Value, const FLinearColor& Color)
{
	UTextRenderComponent* T = NewObject<UTextRenderComponent>(this);
	T->SetupAttachment(RootComponent);
	T->SetUsingAbsoluteLocation(true);
	T->RegisterComponent();
	T->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
	T->SetTextRenderColor(Color.ToFColor(true));
	T->SetWorldSize(38.f);
	T->SetHorizontalAlignment(EHTA_Center);
	T->SetWorldLocation(Pos + FVector(FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-20.f, 20.f), 0.f));
	Texts.Add(T);
	FTimed Timed;
	Timed.Kind = ETimedKind::Text;
	Timed.Comp = T;
	Timed.Duration = 0.7f;
	Timed.StartScale = 1.f;
	Timed.EndScale = 1.f;
	Timed.Rise = 110.f;
	Timed.bFade = false;
	TimedList.Add(Timed);
}

void ASornFx::Beam(const FVector& From, const FVector& To, const FLinearColor& Color, float Duration)
{
	UMaterialInterface* M = Lib(this)->Mat(Color, 0.f, 0.3f, 12.f);
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(Lib(this)->Mesh(EKhonShape::Cylinder));
	C->SetMaterial(0, M);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetupAttachment(RootComponent);
	C->SetUsingAbsoluteLocation(true);
	C->SetUsingAbsoluteRotation(true);
	C->SetUsingAbsoluteScale(true);
	C->RegisterComponent();
	const FVector Dir = To - From;
	C->SetWorldLocation((From + To) * 0.5f);
	C->SetWorldRotation(FRotationMatrix::MakeFromZ(Dir.GetSafeNormal()).Rotator());
	C->SetWorldScale3D(FVector(0.25f, 0.25f, Dir.Size() / 100.f));
	FTimed T;
	T.Kind = ETimedKind::Beam;
	T.Comp = C;
	T.Duration = Duration;
	T.StartScale = 0.25f;
	T.EndScale = 0.02f;
	T.bFade = false;
	TimedList.Add(T);
}

void ASornFx::UpdateTimed(float Dt)
{
	for (int32 I = TimedList.Num() - 1; I >= 0; --I)
	{
		FTimed& T = TimedList[I];
		T.Age += Dt;
		USceneComponent* C = T.Comp.Get();
		if (!C || T.Age >= T.Duration)
		{
			if (C)
			{
				TArray<USceneComponent*> Kids;
				C->GetChildrenComponents(true, Kids);
				for (USceneComponent* K : Kids)
				{
					K->DestroyComponent();
				}
				C->DestroyComponent();
			}
			TimedList.RemoveAtSwap(I);
			continue;
		}
		const float U = T.Age / T.Duration;
		const float Ease = 1.f - FMath::Pow(1.f - U, 3.f);
		switch (T.Kind)
		{
		case ETimedKind::Text:
			if (UTextRenderComponent* Txt = Cast<UTextRenderComponent>(C))
			{
				Txt->AddWorldOffset(FVector(0, 0, T.Rise * Dt / T.Duration));
				if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
				{
					const FVector To = Cam->GetCameraLocation() - Txt->GetComponentLocation();
					Txt->SetWorldRotation(FRotator(0.f, FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X)), 0.f));
				}
			}
			break;
		case ETimedKind::Disc:
		{
			const float S = FMath::Lerp(T.StartScale, T.EndScale, U);
			C->SetWorldScale3D(FVector(S, S, 0.02f));
			break;
		}
		case ETimedKind::Beam:
		{
			const float S = FMath::Lerp(T.StartScale, T.EndScale, U);
			const FVector Cur = C->GetComponentScale();
			C->SetWorldScale3D(FVector(S, S, Cur.Z));
			break;
		}
		default:
		{
			const float S = FMath::Lerp(T.StartScale, T.EndScale, Ease);
			C->SetWorldScale3D(FVector(S, S, 1.f));
			break;
		}
		}
		if (T.bFade && T.Mat.IsValid())
		{
			UKhonMaterialSubsystem::SetMatColor(T.Mat.Get(), T.Color, 1.f - U);
		}
	}
}

void ASornFx::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UWorld* World = GetWorld();
	const float RealDt = World->DeltaRealTimeSeconds;
	const bool bStopped = FPlatformTime::Seconds() < HitStopUntil;
	UGameplayStatics::SetGlobalTimeDilation(this, bStopped ? 0.04f : SlowMo);

	const float Dt = DeltaSeconds;
	for (int32 I = SparkList.Num() - 1; I >= 0; --I)
	{
		FSpark& S = SparkList[I];
		S.Life -= Dt;
		UStaticMeshComponent* C = S.Comp.Get();
		if (!C || S.Life <= 0.f)
		{
			if (C)
			{
				C->DestroyComponent();
			}
			SparkList.RemoveAtSwap(I);
			continue;
		}
		S.Vel.Z -= 1400.f * Dt;
		C->AddWorldOffset(S.Vel * Dt);
	}
	UpdateTimed(RealDt);
}

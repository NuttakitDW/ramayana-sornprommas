#include "ArenaBuilder.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "KhonParts.h"
#include "SornTypes.h"

using namespace KhonParts;

namespace
{
	constexpr float R = Sorn::ArenaRadius / 100.f; // metres
}

AArenaBuilder::AArenaBuilder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AArenaBuilder::BeginPlay()
{
	Super::BeginPlay();
	Environment();
	Ground();
	StageRing();
	Standards();
	Torches();
	Rocks();
	LankaSkyline();
	Hills();
}

void AArenaBuilder::Environment()
{
	UWorld* W = GetWorld();
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Low dusk sun in the south-west so figures cast long shadows toward Lanka.
	ADirectionalLight* Sun = W->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FTransform::Identity, P);
	Sun->SetMobility(EComponentMobility::Movable);
	if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
	{
		L->SetIntensity(6.f);
		L->SetLightColor(FLinearColor(1.f, 0.8f, 0.58f));
		L->SetAtmosphereSunLight(true);
		L->SetCastShadows(true);
	}
	Sun->SetActorRotation(FRotator(-14.f, 35.f, 0.f));

	AActor* Sky = W->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, P);
	USkyAtmosphereComponent* Atmos = NewObject<USkyAtmosphereComponent>(Sky);
	Sky->SetRootComponent(Atmos);
	Atmos->RegisterComponent();

	ASkyLight* SkyLight = W->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FTransform::Identity, P);
	if (USkyLightComponent* SL = SkyLight->GetLightComponent())
	{
		SL->SetMobility(EComponentMobility::Movable);
		SL->bRealTimeCapture = true;
		SL->SetIntensity(1.2f);
		SL->RecaptureSky();
	}

	AExponentialHeightFog* Fog = W->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FTransform::Identity, P);
	if (UExponentialHeightFogComponent* F = Fog->GetComponent())
	{
		F->SetFogDensity(0.02f);
		F->SetFogHeightFalloff(0.08f);
		F->SetFogInscatteringColor(FLinearColor(0.72f, 0.5f, 0.42f));
		F->SetStartDistance(1500.f);
	}

	APostProcessVolume* PPV = W->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, P);
	PPV->bUnbound = true;
	FPostProcessSettings& S = PPV->Settings;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = 0.8f;
	S.bOverride_AutoExposureMinBrightness = true;
	S.AutoExposureMinBrightness = 1.f;
	S.bOverride_AutoExposureMaxBrightness = true;
	S.AutoExposureMaxBrightness = 3.f;
	S.bOverride_VignetteIntensity = true;
	S.VignetteIntensity = 0.45f;
	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.f);
}

void AArenaBuilder::Ground()
{
	UStaticMeshComponent* G = NewObject<UStaticMeshComponent>(this, TEXT("Ground"));
	G->SetStaticMesh(Lib(this)->Mesh(EKhonShape::Cube));
	G->SetMaterial(0, Lib(this)->Mat(Sorn::Col::Earth, 0.f, 0.95f));
	G->SetupAttachment(RootComponent);
	G->SetRelativeLocation(FVector(0, 0, -50.f));
	G->SetRelativeScale3D(FVector(300.f, 300.f, 1.f));
	G->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	G->RegisterComponent();

	// Darker patches of trampled earth break up the flat ground.
	FRandomStream Rng(21);
	UMaterialInterface* Dark = Lib(this)->Mat(Sorn::Col::EarthDark, 0.f, 1.f);
	UMaterialInterface* Light = Lib(this)->Mat(Sorn::Col::Earth * 1.15f, 0.f, 1.f);
	for (int32 I = 0; I < 70; ++I)
	{
		const float A = Rng.FRandRange(0.f, UE_TWO_PI);
		const float D = Rng.FRandRange(0.f, 60.f);
		const float S = Rng.FRandRange(3.f, 12.f);
		Part(this, RootComponent, EKhonShape::Cylinder, I % 3 == 0 ? Light : Dark,
			FVector(FMath::Cos(A) * D, 0.005f + I * 0.0002f, FMath::Sin(A) * D), FVector(S, 0.01f, S * Rng.FRandRange(0.5f, 1.f)),
			FVector(0, Rng.FRandRange(0.f, 360.f), 0));
	}
}

void AArenaBuilder::StageRing()
{
	UMaterialInterface* GoldM = Lib(this)->Mat(Sorn::Col::GoldDeep, 0.7f, 0.4f);
	UMaterialInterface* Stone = Lib(this)->Mat(Sorn::Col::Stone, 0.f, 0.9f);
	for (const float Radius : {R, 4.9f})
	{
		const int32 N = Radius > 10.f ? 96 : 32;
		const float Seg = UE_TWO_PI * Radius / N * 1.05f;
		for (int32 I = 0; I < N; ++I)
		{
			const float A = UE_TWO_PI * I / N;
			Part(this, RootComponent, EKhonShape::Cube, GoldM, FVector(FMath::Cos(A) * Radius, 0.02f, FMath::Sin(A) * Radius),
				FVector(0.35f, 0.04f, Seg), FVector(0, -FMath::RadiansToDegrees(A), 0));
		}
	}
	// Eight gilded lotus-bud markers on the ring.
	for (int32 I = 0; I < 8; ++I)
	{
		const float A = UE_TWO_PI * I / 8;
		const FVector Pos(FMath::Cos(A) * R, 0, FMath::Sin(A) * R);
		Part(this, RootComponent, EKhonShape::Cylinder, Stone, Pos + FVector(0, 0.25f, 0), FVector(0.6, 0.5, 0.6));
		Part(this, RootComponent, EKhonShape::Sphere, GoldM, Pos + FVector(0, 0.7f, 0), FVector(0.44, 0.5, 0.44));
	}
}

/** ธงชัย (battle pennants) and ฉัตร (tiered parasols) ring the field. */
void AArenaBuilder::Standards()
{
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* Pole = L->Mat(Sorn::Col::KhonRedDark, 0.2f, 0.6f);
	UMaterialInterface* GoldM = Gold(this);
	UMaterialInterface* White = L->Mat(FLinearColor::White, 0.f, 0.5f);
	for (int32 I = 0; I < 12; ++I)
	{
		const float A = UE_TWO_PI * (I + 0.5f) / 12.f;
		const FVector Pos(FMath::Cos(A) * (R + 3.5f), 0, FMath::Sin(A) * (R + 3.5f));
		Part(this, RootComponent, EKhonShape::Cylinder, Pole, Pos + FVector(0, 3.5f, 0), FVector(0.14, 7.0, 0.14));
		Part(this, RootComponent, EKhonShape::Cone, GoldM, Pos + FVector(0, 7.25f, 0), FVector(0.24, 0.5, 0.24));
		if (I % 3 == 0)
		{
			for (int32 T = 0; T < 5; ++T)
			{
				const float D = 2.f * (1.1f - T * 0.18f);
				Part(this, RootComponent, EKhonShape::Cylinder, T % 2 == 0 ? GoldM : White, Pos + FVector(0, 4.6f + T * 0.45f, 0), FVector(D, 0.18, D));
			}
		}
		else
		{
			const FLinearColor FlagCol = I % 2 == 0 ? Sorn::Col::KhonRed : Sorn::Col::UiDepth;
			// Rectangular banner hanging from the pole, spanning along the ring.
			const FVector Tangent(-FMath::Sin(A), 0, FMath::Cos(A));
			Part(this, RootComponent, EKhonShape::Cube, L->Mat(FlagCol, 0.f, 0.6f), Pos + FVector(0, 5.6f, 0) + Tangent * 0.62f,
				FVector(0.04, 1.8, 1.2), FVector(0, -FMath::RadiansToDegrees(A), 0));
			Part(this, RootComponent, EKhonShape::Cube, GoldM, Pos + FVector(0, 6.55f, 0) + Tangent * 0.62f,
				FVector(0.06, 0.08, 1.3), FVector(0, -FMath::RadiansToDegrees(A), 0));
		}
	}
}

void AArenaBuilder::Torches()
{
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* Flame = L->Mat(FLinearColor(1.f, 0.55f, 0.18f), 0.f, 0.4f, 25.f);
	UMaterialInterface* Wood = L->Mat(Sorn::Col::EarthDark, 0.f, 0.9f);
	for (int32 I = 0; I < 6; ++I)
	{
		const float A = UE_TWO_PI * I / 6.f + 0.3f;
		const FVector Pos(FMath::Cos(A) * (R + 1.6f), 0, FMath::Sin(A) * (R + 1.6f));
		Part(this, RootComponent, EKhonShape::Cylinder, Wood, Pos + FVector(0, 1.1f, 0), FVector(0.17, 2.2, 0.17));
		Part(this, RootComponent, EKhonShape::Cylinder, Gold(this), Pos + FVector(0, 2.25f, 0), FVector(0.4, 0.25, 0.4));
		Part(this, RootComponent, EKhonShape::Cone, Flame, Pos + FVector(0, 2.6f, 0), FVector(0.36, 0.5, 0.36));
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetRelativeLocation(Sorn::G(Pos.X, 2.8f, Pos.Z));
		Light->SetLightColor(FLinearColor(1.f, 0.62f, 0.3f));
		Light->SetIntensity(8000.f);
		Light->SetAttenuationRadius(900.f);
		Light->SetCastShadows(false);
		Light->RegisterComponent();
	}
}

void AArenaBuilder::Rocks()
{
	FRandomStream Rng(7);
	UMaterialInterface* Stone = Lib(this)->Mat(Sorn::Col::Stone, 0.f, 0.95f);
	for (int32 I = 0; I < 26; ++I)
	{
		const float A = Rng.FRandRange(0.f, UE_TWO_PI);
		const float D = Rng.FRandRange(R + 5.f, R + 16.f);
		const float S = Rng.FRandRange(0.5f, 2.2f);
		Part(this, RootComponent, EKhonShape::Sphere, Stone, FVector(FMath::Cos(A) * D, S * 0.2f, FMath::Sin(A) * D),
			FVector(S * 2.f, S * 1.1f, S * 1.6f), FVector(Rng.FRandRange(0.f, 40.f), Rng.FRandRange(0.f, 360.f), 0));
	}
}

/** กรุงลงกา on the northern horizon: a long wall with prang towers. */
void AArenaBuilder::LankaSkyline()
{
	UKhonMaterialSubsystem* L = Lib(this);
	UMaterialInterface* Wall = L->Mat(FLinearColor(0.28f, 0.2f, 0.2f), 0.f, 0.9f);
	UMaterialInterface* GoldM = L->Mat(Sorn::Col::GoldDeep, 0.6f, 0.5f);
	UMaterialInterface* Glow = L->Mat(FLinearColor(1.f, 0.6f, 0.25f), 0.f, 0.5f, 20.f);
	const FVector Base(0, 0, -95);
	Part(this, RootComponent, EKhonShape::Cube, Wall, Base + FVector(0, 3.5f, 0), FVector(140, 7, 3));
	for (int32 I = 0; I < 15; ++I)
	{
		const float X = -63.f + I * 9.f;
		Part(this, RootComponent, EKhonShape::Cube, Wall, Base + FVector(X, 4.5f, 0), FVector(2.2, 9, 3.6));
		Part(this, RootComponent, EKhonShape::Cube, Glow, Base + FVector(X, 5.5f, 1.85f), FVector(0.5, 0.8, 0.1));
	}
	const FVector2D Prangs[] = {{-30.f, 26.f}, {-12.f, 34.f}, {0.f, 46.f}, {14.f, 32.f}, {32.f, 24.f}};
	for (const FVector2D& Pr : Prangs)
	{
		const float H = Pr.Y;
		const FVector Pos = Base + FVector(Pr.X, 0, -10);
		Part(this, RootComponent, EKhonShape::Cube, Wall, Pos + FVector(0, H * 0.15f, 0), FVector(H * 0.3f));
		Part(this, RootComponent, EKhonShape::Cylinder, Wall, Pos + FVector(0, H * 0.6f, 0), FVector(H * 0.19f, H * 0.6f, H * 0.19f));
		Part(this, RootComponent, EKhonShape::Cone, GoldM, Pos + FVector(0, H * 1.02f, 0), FVector(H * 0.1f, H * 0.25f, H * 0.1f));
	}
}

void AArenaBuilder::Hills()
{
	FRandomStream Rng(11);
	UMaterialInterface* Far = Lib(this)->Mat(FLinearColor(0.34f, 0.28f, 0.38f), 0.f, 1.f);
	for (int32 I = 0; I < 18; ++I)
	{
		const float A = UE_TWO_PI * I / 18.f + Rng.FRandRange(0.f, 0.15f);
		if (FMath::Abs(FMath::UnwindRadians(A - 1.5f * UE_PI)) < 0.55f)
		{
			continue; // keep the Lanka view open
		}
		const float D = Rng.FRandRange(150.f, 190.f);
		const float W = Rng.FRandRange(30.f, 55.f);
		const float H = Rng.FRandRange(10.f, 22.f);
		Part(this, RootComponent, EKhonShape::Sphere, Far, FVector(FMath::Cos(A) * D, -H * 0.25f, FMath::Sin(A) * D),
			FVector(W * 2.f, H * 2.f, W * 1.4f), FVector(0, Rng.FRandRange(0.f, 360.f), 0));
	}
}

#include "SornHUD.h"

#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "ErawanBoss.h"
#include "MusicDirector.h"
#include "SSornHudWidget.h"
#include "SornGameMode.h"
#include "Widgets/SWeakWidget.h"

ASornHUD::ASornHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void ASornHUD::BeginPlay()
{
	Super::BeginPlay();
	// Thai-capable OFL fonts (Taviraj, Sarabun, IBM Plex Mono) shipped in RawFonts/.
	auto Resolve = [](const TCHAR* File)
	{
		const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("RawFonts") / File);
		if (!FPaths::FileExists(Path))
		{
			UE_LOG(LogTemp, Warning, TEXT("Font not found: %s (falling back to engine font)"), *Path);
			return FString();
		}
		return Path;
	};
	DisplayFontPath = Resolve(TEXT("Taviraj-SemiBold.ttf"));
	TextFontPath = Resolve(TEXT("Sarabun-Regular.ttf"));
	MonoFontPath = Resolve(TEXT("IBMPlexMono-Regular.ttf"));

	if (GEngine && GEngine->GameViewport)
	{
		Widget = SNew(SSornHudWidget).Hud(this);
		Root = SNew(SWeakWidget).PossiblyNullContent(Widget);
		GEngine->GameViewport->AddViewportWidgetContent(Root.ToSharedRef(), 10);
	}
}

void ASornHUD::EndPlay(const EEndPlayReason::Type Reason)
{
	if (GEngine && GEngine->GameViewport && Root.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Root.ToSharedRef());
	}
	Widget.Reset();
	Root.Reset();
	Super::EndPlay(Reason);
}

void ASornHUD::OnBeat(int32 /*Index*/, bool bIsChap)
{
	BeatLight[bIsChap ? 1 : 0] = 1.f;
}

void ASornHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Dt = GetWorld()->DeltaRealTimeSeconds;
	if (!bBeatBound)
	{
		if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
		{
			GM->Music->OnBeat.AddUObject(this, &ASornHUD::OnBeat);
			bBeatBound = true;
		}
	}
	BannerLeft -= Dt;
	BannerAlpha = FMath::FInterpConstantTo(BannerAlpha, BannerLeft > 0.f ? 1.f : 0.f, Dt, BannerLeft > 0.f ? 3.f : 1.6f);
	CardAlpha = FMath::FInterpConstantTo(CardAlpha, CardTarget, Dt, 3.f);
	Vignette = FMath::FInterpConstantTo(Vignette, 0.f, Dt, 0.8f);
	for (float& B : BeatLight)
	{
		B = FMath::FInterpConstantTo(B, 0.f, Dt, 2.5f);
	}
	if (FadeT < 1.f)
	{
		FadeT = FMath::Min(1.f, FadeT + Dt / FadeDur);
		FadeAlpha = FMath::Lerp(FadeFrom, FadeTarget, FadeT);
	}
}

void ASornHUD::SetObjective(const FString& Th, const FString& En)
{
	ObjectiveTh = Th;
	ObjectiveEn = En;
}

void ASornHUD::ShowBanner(const FSornBanner& InBanner, float Seconds)
{
	Banner = InBanner;
	BannerLeft = Seconds;
}

void ASornHUD::ShowCard(const FSornCard& InCard, bool bWaitForConfirm)
{
	Card = InCard;
	bCardWaiting = bWaitForConfirm;
	CardTarget = 1.f;
}

void ASornHUD::HideCard()
{
	bCardWaiting = false;
	CardTarget = 0.f;
}

void ASornHUD::ShowBoss(AErawanBoss* InBoss)
{
	Boss = InBoss;
	bHelp = false;
}

void ASornHUD::HideBoss()
{
	Boss = nullptr;
}

void ASornHUD::FadeTo(float Alpha, float Seconds)
{
	FadeFrom = FadeAlpha;
	FadeTarget = Alpha;
	FadeT = 0.f;
	FadeDur = FMath::Max(Seconds, 0.01f);
}

void ASornHUD::HurtFlash()
{
	Vignette = 0.28f;
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StoryText.h"
#include "SornHUD.generated.h"

class AErawanBoss;
class SSornHudWidget;
class SWidget;

/**
 * Owns the Slate HUD: Hanuman's vitals, objective, ปี่พาทย์ cue panel with a
 * ฉิ่ง/ฉับ beat light, combo, boss bar, verse banner, story cards, fades.
 * Slate text uses the engine's shaping, so Thai renders correctly with a Thai font.
 */
UCLASS()
class SORNPROMMAS_API ASornHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASornHUD();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaSeconds) override;

	void SetObjective(const FString& Th, const FString& En);
	void ShowBanner(const FSornBanner& Banner, float Seconds);
	void ShowCard(const FSornCard& Card, bool bWaitForConfirm);
	void HideCard();
	void ShowBoss(AErawanBoss* Boss);
	void HideBoss();
	void FadeTo(float Alpha, float Seconds);
	void HurtFlash();
	void ToggleHelp() { bHelp = !bHelp; }

	// State read by the widget.
	FString ObjectiveTh, ObjectiveEn;
	FSornBanner Banner;
	float BannerAlpha = 0.f;
	FSornCard Card;
	float CardAlpha = 0.f;
	bool bCardWaiting = false;
	bool bHelp = true;
	float FadeAlpha = 0.f;
	float Vignette = 0.f;
	float BeatLight[2] = {0.f, 0.f};
	TWeakObjectPtr<AErawanBoss> Boss;

	/** OFL fonts shipped in RawFonts/ and loaded from disk at runtime; empty if missing. */
	FString DisplayFontPath;
	FString TextFontPath;
	FString MonoFontPath;

private:
	void OnBeat(int32 Index, bool bIsChap);

	TSharedPtr<SSornHudWidget> Widget;
	TSharedPtr<SWidget> Root;
	float BannerLeft = 0.f;
	float CardTarget = 0.f;
	float FadeFrom = 0.f;
	float FadeTarget = 0.f;
	float FadeT = 1.f;
	float FadeDur = 1.f;
	bool bBeatBound = false;
};

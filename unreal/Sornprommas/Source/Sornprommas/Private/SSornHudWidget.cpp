#include "SSornHudWidget.h"

#include "ErawanBoss.h"
#include "HanumanCharacter.h"
#include "MusicDirector.h"
#include "SornGameMode.h"
#include "SornHUD.h"
#include "SornTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

using namespace Sorn::Col;

namespace
{
	FText T(const FString& S) { return FText::FromString(S); }

	FString Join(const TArray<FString>& Lines) { return FString::Join(Lines, TEXT("\n")); }

	FLinearColor A(const FLinearColor& C, float Alpha) { return FLinearColor(C.R, C.G, C.B, Alpha); }
}

FSlateFontInfo SSornHudWidget::Font(EFont Kind, int32 Size) const
{
	const ASornHUD* H = Hud.Get();
	FString Path;
	if (H)
	{
		Path = Kind == EFont::Display ? H->DisplayFontPath : Kind == EFont::Mono ? H->MonoFontPath : H->TextFontPath;
	}
	if (!Path.IsEmpty())
	{
		// Legacy path-based constructor: builds a runtime composite font from the TTF.
		return FSlateFontInfo(Path, Size);
	}
	return FCoreStyle::GetDefaultFontStyle("Regular", Size);
}

TSharedRef<SWidget> SSornHudWidget::Label(TAttribute<FText> Text, EFont Kind, int32 Size, TAttribute<FSlateColor> Color,
	ETextJustify::Type Justify) const
{
	return SNew(STextBlock)
		.Text(Text)
		.Font(Font(Kind, Size))
		.ColorAndOpacity(Color)
		.Justification(Justify)
		.ShadowOffset(FVector2D(0.f, 2.f))
		.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.55f));
}

TSharedRef<SWidget> SSornHudWidget::Bar(TAttribute<float> Ratio, float Width, float Height, const FLinearColor& Fill) const
{
	return SNew(SBorder)
		.BorderImage(&White)
		.BorderBackgroundColor(UiEdge)
		.Padding(2.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBox).WidthOverride(Width).HeightOverride(Height)
				[
					SNew(SImage).Image(&White).ColorAndOpacity(A(UiInk, 0.9f))
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left)
			[
				SNew(SBox)
				.HeightOverride(Height)
				.WidthOverride(TAttribute<FOptionalSize>::CreateLambda([Ratio, Width] { return FOptionalSize(Width * FMath::Clamp(Ratio.Get(), 0.f, 1.f)); }))
				[
					SNew(SImage).Image(&White).ColorAndOpacity(Fill)
				]
			]
		];
}

void SSornHudWidget::Construct(const FArguments& InArgs)
{
	Hud = InArgs._Hud;
	TWeakObjectPtr<ASornHUD> W = Hud;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage).Image(&White).Visibility(EVisibility::HitTestInvisible)
			.ColorAndOpacity_Lambda([W] { return A(Danger, W.IsValid() ? W->Vignette : 0.f); })
		]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(36.f, 14.f) [ PlayerPanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 20.f) [ ObjectivePanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 20.f, 30.f, 0.f) [ CuePanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).Padding(0.f, 0.f, 40.f, 80.f) [ ComboPanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 36.f) [ BossPanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 170.f) [ BannerPanel() ]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(36.f, 0.f, 0.f, 30.f) [ HelpPanel() ]
		+ SOverlay::Slot()
		[
			SNew(SImage).Image(&White).Visibility(EVisibility::HitTestInvisible)
			.ColorAndOpacity_Lambda([W] { return FLinearColor(0.f, 0.f, 0.f, W.IsValid() ? W->FadeAlpha : 0.f); })
		]
		+ SOverlay::Slot() [ CardPanel() ]
	];
}

TSharedRef<SWidget> SSornHudWidget::PlayerPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	auto Hero = [W]() -> AHanumanCharacter*
	{
		const ASornGameMode* GM = W.IsValid() ? ASornGameMode::Get(W.Get()) : nullptr;
		return GM ? GM->Hanuman() : nullptr;
	};
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight() [ Label(T(TEXT("หนุมาน")), EFont::Display, 32, UiPale) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.f, 0.f, 0.f, 8.f)
		[ Label(T(TEXT("HANUMAN / วายุบุตร  SON OF THE WIND")), EFont::Text, 10, UiBronze) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[ Bar(TAttribute<float>::CreateLambda([Hero] { const AHanumanCharacter* H = Hero(); return H ? H->Hp / AHanumanCharacter::MaxHp : 0.f; }), 380.f, 14.f, UiHp) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f)
			[
				Label(TAttribute<FText>::CreateLambda([Hero]
				{
					const AHanumanCharacter* H = Hero();
					return T(H ? FString::Printf(TEXT("HP %3d/%d"), FMath::RoundToInt(H->Hp), FMath::RoundToInt(AHanumanCharacter::MaxHp)) : FString());
				}), EFont::Mono, 10, UiPale)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 4.f)
		[ Bar(TAttribute<float>::CreateLambda([Hero] { const AHanumanCharacter* H = Hero(); return H ? H->Power / 100.f : 0.f; }), 380.f, 8.f, UiSignal) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			Label(TAttribute<FText>::CreateLambda([Hero]
			{
				const AHanumanCharacter* H = Hero();
				if (!H) return FText::GetEmpty();
				if (H->GiantLeft > 0.f) return T(FString::Printf(TEXT("นิมิตกาย  NIMIT KAI  %.1fs"), H->GiantLeft));
				if (H->Power >= 100.f) return T(TEXT("นิมิตกาย  NIMIT KAI  READY  [Q]"));
				return T(FString::Printf(TEXT("นิมิตกาย  NIMIT KAI  %3d%%"), FMath::RoundToInt(H->Power)));
			}), EFont::Text, 11,
			TAttribute<FSlateColor>::CreateLambda([Hero]
			{
				const AHanumanCharacter* H = Hero();
				if (H && (H->GiantLeft > 0.f || H->Power >= 100.f))
				{
					return FSlateColor(A(UiSignal, 0.7f + 0.3f * FMath::Sin(FPlatformTime::Seconds() * 8.0)));
				}
				return FSlateColor(UiAsh);
			}))
		];
}

TSharedRef<SWidget> SSornHudWidget::ObjectivePanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ Label(TAttribute<FText>::CreateLambda([W] { return T(W.IsValid() ? W->ObjectiveTh : FString()); }), EFont::Text, 18, UiPale, ETextJustify::Center) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ Label(TAttribute<FText>::CreateLambda([W] { return T(W.IsValid() ? W->ObjectiveEn : FString()); }), EFont::Mono, 10, UiBronze, ETextJustify::Center) ];
}

TSharedRef<SWidget> SSornHudWidget::CuePanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	auto Music = [W]() -> AMusicDirector*
	{
		const ASornGameMode* GM = W.IsValid() ? ASornGameMode::Get(W.Get()) : nullptr;
		return GM ? GM->Music.Get() : nullptr;
	};
	auto Info = [Music]() -> const FMusicCue*
	{
		const AMusicDirector* M = Music();
		return M ? M->CurrentInfo() : nullptr;
	};
	auto Beat = [W](int32 I, const FLinearColor& On)
	{
		return TAttribute<FSlateColor>::CreateLambda([W, I, On]
		{
			const float L = W.IsValid() ? W->BeatLight[I] : 0.f;
			return FSlateColor(FMath::Lerp(A(UiAsh, 0.5f), On, L));
		});
	};
	return SNew(SBox).WidthOverride(310.f)
		[
			SNew(SBorder).BorderImage(&White).BorderBackgroundColor(A(UiPanel, 0.72f)).Padding(FMargin(20.f, 14.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight() [ Label(T(TEXT("PI PHAT CUE / เพลงปี่พาทย์")), EFont::Text, 9, UiBronze) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ Label(TAttribute<FText>::CreateLambda([Info] { const FMusicCue* I = Info(); return T(I ? I->Th : TEXT("-")); }), EFont::Display, 22, UiPale) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ Label(TAttribute<FText>::CreateLambda([Info] { const FMusicCue* I = Info(); return T(I ? FString::Printf(TEXT("%s   %d BPM"), *I->Rom, I->Bpm) : FString()); }), EFont::Mono, 9, UiSignal) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 14.f, 0.f) [ Label(T(TEXT("ฉิ่ง")), EFont::Text, 12, Beat(0, UiBronze)) ]
					+ SHorizontalBox::Slot().AutoWidth() [ Label(T(TEXT("ฉับ")), EFont::Text, 12, Beat(1, UiSignal)) ]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					Label(TAttribute<FText>::CreateLambda([Info, Music]
					{
						const FMusicCue* I = Info();
						const AMusicDirector* M = Music();
						if (!I) return FText::GetEmpty();
						return T(M && M->bUsingRecording ? I->Use : I->Use + TEXT("\nplaceholder percussion, not the real piece"));
					}), EFont::Text, 9, UiAsh)
				]
			]
		];
}

TSharedRef<SWidget> SSornHudWidget::ComboPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	auto Combo = [W]() -> int32
	{
		const ASornGameMode* GM = W.IsValid() ? ASornGameMode::Get(W.Get()) : nullptr;
		const AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr;
		return H ? H->ComboCount : 0;
	};
	return SNew(SVerticalBox)
		.Visibility_Lambda([Combo] { return Combo() >= 2 ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[ Label(TAttribute<FText>::CreateLambda([Combo] { return FText::AsNumber(Combo()); }), EFont::Mono, 40, UiPale, ETextJustify::Right) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[ Label(T(TEXT("HITS / ครั้ง")), EFont::Text, 10, UiBronze, ETextJustify::Right) ];
}

TSharedRef<SWidget> SSornHudWidget::BossPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	TSharedRef<SHorizontalBox> Bars = SNew(SHorizontalBox);
	for (int32 I = 0; I < 3; ++I)
	{
		Bars->AddSlot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[ Bar(TAttribute<float>::CreateLambda([W, I] { return W.IsValid() && W->Boss.IsValid() ? W->Boss->HeadHp[I] / AErawanBoss::HeadMaxHp : 0.f; }), 248.f, 12.f, Gold) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
			[ Label(T(FString::Printf(TEXT("เศียร %d / HEAD %d"), I + 1, I + 1)), EFont::Text, 8, UiAsh) ]
		];
	}
	return SNew(SVerticalBox)
		.Visibility_Lambda([W] { return W.IsValid() && W->Boss.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		+ SVerticalBox::Slot().AutoHeight() [ Label(T(TEXT("อินทรชิตแปลงเป็นพระอินทร์ ทรงช้างเอราวัณ")), EFont::Text, 16, UiPale) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[ Label(T(TEXT("INDRAJIT DISGUISED AS INDRA, ON ERAWAN (THE DEMON KARUNARAT TRANSFORMED)")), EFont::Mono, 8, UiBronze) ]
		+ SVerticalBox::Slot().AutoHeight() [ Bars ];
}

TSharedRef<SWidget> SSornHudWidget::BannerPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	return SNew(SBox).WidthOverride(920.f)
		.Visibility_Lambda([W] { return W.IsValid() && W->BannerAlpha > 0.01f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		[
			SNew(SBorder).BorderImage(&White).Padding(FMargin(22.f, 14.f))
			.BorderBackgroundColor_Lambda([W] { return FSlateColor(A(UiPanel, 0.8f * (W.IsValid() ? W->BannerAlpha : 0.f))); })
			.ColorAndOpacity_Lambda([W] { return FLinearColor(1.f, 1.f, 1.f, W.IsValid() ? W->BannerAlpha : 0.f); })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[ Label(TAttribute<FText>::CreateLambda([W] { return T(W.IsValid() ? Join(W->Banner.Lines) : FString()); }), EFont::Text, 16, UiPale, ETextJustify::Center) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[ Label(TAttribute<FText>::CreateLambda([W] { return T(W.IsValid() ? W->Banner.Source : FString()); }), EFont::Text, 8, UiBronze, ETextJustify::Center) ]
			]
		];
}

TSharedRef<SWidget> SSornHudWidget::HelpPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	const FString Help = FString::Join(TArray<FString>{
		TEXT("WASD      move            MOUSE / ARROWS  camera"),
		TEXT("J / LMB   trident combo   K / RMB         heavy thrust"),
		TEXT("SPACE     leap            J in air        plunge"),
		TEXT("SHIFT     dash            Q               grow (NIMIT KAI)"),
		TEXT("R         restart         H               hide help"),
	}, TEXT("\n"));
	return SNew(SBox)
		.Visibility_Lambda([W] { return W.IsValid() && W->bHelp ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		[ Label(T(Help), EFont::Mono, 9, UiAsh) ];
}

TSharedRef<SWidget> SSornHudWidget::CardPanel()
{
	TWeakObjectPtr<ASornHUD> W = Hud;
	auto Field = [W](FString FSornCard::*Member)
	{
		return TAttribute<FText>::CreateLambda([W, Member] { return T(W.IsValid() ? W->Card.*Member : FString()); });
	};
	return SNew(SBorder).BorderImage(&White)
		.Visibility_Lambda([W] { return W.IsValid() && W->CardAlpha > 0.01f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		.BorderBackgroundColor_Lambda([W] { return FSlateColor(A(UiInk, 0.55f * (W.IsValid() ? W->CardAlpha : 0.f))); })
		.ColorAndOpacity_Lambda([W] { return FLinearColor(1.f, 1.f, 1.f, W.IsValid() ? W->CardAlpha : 0.f); })
		.HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(1040.f)
			[
				SNew(SBorder).BorderImage(&White).BorderBackgroundColor(UiBronze).Padding(FMargin(1.f, 2.f, 1.f, 1.f))
				[
					SNew(SBorder).BorderImage(&White).BorderBackgroundColor(A(UiPanel, 0.92f)).Padding(FMargin(28.f, 20.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ Label(Field(&FSornCard::Kicker), EFont::Text, 10, UiBronze, ETextJustify::Center) ]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f)
						[
							SNew(SBox).Visibility_Lambda([W] { return W.IsValid() && !W->Card.Title.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
							[ Label(Field(&FSornCard::Title), EFont::Display, 44, UiPale, ETextJustify::Center) ]
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ Label(Field(&FSornCard::Subtitle), EFont::Mono, 11, UiSignal, ETextJustify::Center) ]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f)
						[ SNew(SBox).HeightOverride(1.f) [ SNew(SImage).Image(&White).ColorAndOpacity(A(UiBronze, 0.6f)) ] ]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[ Label(TAttribute<FText>::CreateLambda([W] { return T(W.IsValid() ? Join(W->Card.Body) : FString()); }), EFont::Text, 16, UiPale, ETextJustify::Center) ]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f) [ Label(Field(&FSornCard::Source), EFont::Text, 9, UiBronze, ETextJustify::Center) ]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 4.f)
						[
							Label(Field(&FSornCard::Prompt), EFont::Text, 11, TAttribute<FSlateColor>::CreateLambda([W]
							{
								const bool bBlink = W.IsValid() && W->bCardWaiting;
								return FSlateColor(A(UiPale, bBlink ? 0.55f + 0.45f * FMath::Sin(FPlatformTime::Seconds() * 4.0) : 1.f));
							}), ETextJustify::Center)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ Label(Field(&FSornCard::Footer), EFont::Mono, 8, UiAsh, ETextJustify::Center) ]
					]
				]
			]
		];
}

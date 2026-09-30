#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SCompoundWidget.h"

class ASornHUD;

/** Slate layout for ASornHUD. All values are pulled from the HUD each frame. */
class SSornHudWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSornHudWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ASornHUD>, Hud)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	enum class EFont : uint8 { Display, Text, Mono };

	FSlateFontInfo Font(EFont Kind, int32 Size) const;
	TSharedRef<SWidget> Label(TAttribute<FText> Text, EFont Kind, int32 Size, TAttribute<FSlateColor> Color,
		ETextJustify::Type Justify = ETextJustify::Left) const;
	TSharedRef<SWidget> Bar(TAttribute<float> Ratio, float Width, float Height, const FLinearColor& Fill) const;

	TSharedRef<SWidget> PlayerPanel();
	TSharedRef<SWidget> ObjectivePanel();
	TSharedRef<SWidget> CuePanel();
	TSharedRef<SWidget> ComboPanel();
	TSharedRef<SWidget> BossPanel();
	TSharedRef<SWidget> BannerPanel();
	TSharedRef<SWidget> HelpPanel();
	TSharedRef<SWidget> CardPanel();

	TWeakObjectPtr<ASornHUD> Hud;
	FSlateColorBrush White = FSlateColorBrush(FLinearColor::White);
};

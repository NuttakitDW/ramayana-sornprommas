#pragma once

#include "CoreMinimal.h"

/**
 * Shared constants for the Sorn Phrommat demo.
 * Character colours follow lore/khon-performance/05-masks-costumes.md;
 * UI colours follow the house identity (sapphire + bronze on near-black).
 *
 * Geometry was designed in metres with Y up and -Z forward (the Godot demo);
 * Sorn::G() converts those to Unreal centimetres (X forward, Y right, Z up).
 */
namespace Sorn
{
	/** Metres (x right, y up, z back) -> Unreal cm (X forward, Y right, Z up). */
	inline FVector G(float X, float Y, float Z) { return FVector(-Z * 100.f, X * 100.f, Y * 100.f); }

	/** Rotation in radians around (right, up, back) axes -> FRotator (pitch, yaw, roll). */
	inline FRotator R(const FVector& Rad)
	{
		return FRotator(FMath::RadiansToDegrees(Rad.X), -FMath::RadiansToDegrees(Rad.Y), FMath::RadiansToDegrees(Rad.Z));
	}

	/** Same as R() but for values already in degrees. */
	inline FRotator RD(float X, float Y, float Z) { return FRotator(X, -Y, Z); }

	constexpr float ArenaRadius = 2000.f;

	namespace Col
	{
		const FLinearColor HanumanWhite(0.95f, 0.94f, 0.90f);
		const FLinearColor IndraGreen(0.18f, 0.50f, 0.30f);
		const FLinearColor ErawanWhite(0.93f, 0.92f, 0.88f);
		const FLinearColor LakshmanGold(0.84f, 0.64f, 0.26f);
		const FLinearColor Gold(0.86f, 0.66f, 0.24f);
		const FLinearColor GoldDeep(0.62f, 0.44f, 0.14f);
		const FLinearColor KhonRed(0.62f, 0.10f, 0.09f);
		const FLinearColor KhonRedDark(0.36f, 0.06f, 0.06f);
		const FLinearColor MouthRed(0.70f, 0.08f, 0.10f);
		const FLinearColor Fang(0.97f, 0.95f, 0.86f);
		const FLinearColor EyeWhite(0.98f, 0.96f, 0.90f);
		const FLinearColor Pupil(0.05f, 0.04f, 0.03f);
		const FLinearColor ClothDark(0.10f, 0.10f, 0.16f);
		const FLinearColor Earth(0.46f, 0.36f, 0.25f);
		const FLinearColor EarthDark(0.30f, 0.23f, 0.16f);
		const FLinearColor Stone(0.42f, 0.40f, 0.38f);
		const FLinearColor Danger(0.95f, 0.18f, 0.10f);

		// UI (dark ground identity)
		const FLinearColor UiInk = FLinearColor::FromSRGBColor(FColor(0x06, 0x08, 0x10));
		const FLinearColor UiPanel = FLinearColor::FromSRGBColor(FColor(0x0B, 0x0E, 0x16));
		const FLinearColor UiEdge = FLinearColor::FromSRGBColor(FColor(0x1F, 0x27, 0x40));
		const FLinearColor UiSignal = FLinearColor::FromSRGBColor(FColor(0x5D, 0x9B, 0xFF));
		const FLinearColor UiDepth = FLinearColor::FromSRGBColor(FColor(0x0F, 0x2A, 0x6B));
		const FLinearColor UiPale = FLinearColor::FromSRGBColor(FColor(0xE8, 0xEC, 0xF5));
		const FLinearColor UiAsh = FLinearColor::FromSRGBColor(FColor(0x8A, 0x93, 0xA8));
		const FLinearColor UiBronze = FLinearColor::FromSRGBColor(FColor(0xB0, 0x8D, 0x4F));
		const FLinearColor UiHp = FLinearColor::FromSRGBColor(FColor(0xC8, 0x41, 0x2F));

		inline FLinearColor YakshaSkin(int32 Index)
		{
			static const FLinearColor Skins[] = {
				FLinearColor(0.16f, 0.42f, 0.22f), // เขียว
				FLinearColor(0.58f, 0.12f, 0.10f), // แดง
				FLinearColor(0.16f, 0.20f, 0.46f), // คราม
				FLinearColor(0.68f, 0.48f, 0.14f), // เหลืองหม่น
				FLinearColor(0.30f, 0.30f, 0.34f), // มอหมึก
			};
			return Skins[FMath::Abs(Index) % UE_ARRAY_COUNT(Skins)];
		}
	}
}

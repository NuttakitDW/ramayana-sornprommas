#pragma once

#include "CoreMinimal.h"

/**
 * Synthesises placeholder percussion as 16-bit mono PCM so the demo has a pulse
 * without shipping recordings. NOT recordings of a ปี่พาทย์ ensemble: rough
 * stand-ins for ฉิ่ง (ching / chap), ตะโพน (taphon hi / lo), กลองทัด (klong)
 * plus a few combat SFX.
 */
namespace PercussionSynth
{
	constexpr int32 SampleRate = 22050;

	/** Kinds: ching, chap, taphon_hi, taphon_lo, klong, whoosh, hit, clang. */
	TArray<uint8> Make(const FString& Kind);
}

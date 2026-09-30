#include "PercussionSynth.h"

namespace
{
	constexpr float Rate = static_cast<float>(PercussionSynth::SampleRate);

	/** Inharmonic partials of a small bronze cymbal pair (ฉิ่ง), Hz. */
	const float ChingPartials[] = {1187.f, 2764.f, 4031.f, 5322.f, 6911.f};

	TArray<float> Ching(float Decay, float NoiseAmt, float Pitch = 1.f)
	{
		const int32 Len = static_cast<int32>(Rate * (Decay * 2.5f + 0.05f));
		TArray<float> Out;
		Out.SetNumZeroed(Len);
		FRandomStream Rng(3);
		for (int32 I = 0; I < Len; ++I)
		{
			const float T = I / Rate;
			float V = 0.f;
			for (int32 K = 0; K < UE_ARRAY_COUNT(ChingPartials); ++K)
			{
				const float D = Decay * (1.f - K * 0.12f);
				V += FMath::Sin(UE_TWO_PI * ChingPartials[K] * Pitch * T) * FMath::Exp(-T / D) / (K + 1.5f);
			}
			V += Rng.FRandRange(-1.f, 1.f) * NoiseAmt * FMath::Exp(-T / 0.02f);
			Out[I] = V * 0.55f;
		}
		return Out;
	}

	TArray<float> Drum(float FStart, float FEnd, float Decay, float Click)
	{
		const int32 Len = static_cast<int32>(Rate * Decay * 3.f);
		TArray<float> Out;
		Out.SetNumZeroed(Len);
		FRandomStream Rng(5);
		float Phase = 0.f;
		for (int32 I = 0; I < Len; ++I)
		{
			const float T = I / Rate;
			const float F = FEnd + (FStart - FEnd) * FMath::Exp(-T / 0.04f);
			Phase += UE_TWO_PI * F / Rate;
			const float Body = FMath::Sin(Phase) + 0.35f * FMath::Sin(Phase * 1.59f) * FMath::Exp(-T / (Decay * 0.4f));
			float V = Body * FMath::Exp(-T / Decay);
			V += Rng.FRandRange(-1.f, 1.f) * Click * FMath::Exp(-T / 0.006f);
			Out[I] = V * 0.7f;
		}
		return Out;
	}

	TArray<float> Whoosh(float Seconds)
	{
		const int32 Len = static_cast<int32>(Rate * Seconds);
		TArray<float> Out;
		Out.SetNumZeroed(Len);
		FRandomStream Rng(9);
		float Lp = 0.f;
		for (int32 I = 0; I < Len; ++I)
		{
			const float U = static_cast<float>(I) / Len;
			const float Cutoff = 0.05f + 0.35f * FMath::Sin(UE_PI * U);
			Lp += (Rng.FRandRange(-1.f, 1.f) - Lp) * Cutoff;
			Out[I] = Lp * FMath::Sin(UE_PI * U) * 0.9f;
		}
		return Out;
	}

	TArray<float> Mix(const TArray<float>& A, const TArray<float>& B, float BGain)
	{
		TArray<float> Out;
		Out.SetNumZeroed(FMath::Max(A.Num(), B.Num()));
		for (int32 I = 0; I < Out.Num(); ++I)
		{
			Out[I] = (I < A.Num() ? A[I] : 0.f) + (I < B.Num() ? B[I] * BGain : 0.f);
		}
		return Out;
	}

	TArray<uint8> ToPcm16(const TArray<float>& Samples)
	{
		TArray<uint8> Bytes;
		Bytes.SetNumUninitialized(Samples.Num() * 2);
		for (int32 I = 0; I < Samples.Num(); ++I)
		{
			const int16 S = static_cast<int16>(FMath::Clamp(Samples[I], -1.f, 1.f) * 32767.f);
			Bytes[I * 2] = static_cast<uint8>(S & 0xFF);
			Bytes[I * 2 + 1] = static_cast<uint8>((S >> 8) & 0xFF);
		}
		return Bytes;
	}
}

namespace PercussionSynth
{
	TArray<uint8> Make(const FString& Kind)
	{
		if (Kind == TEXT("ching")) return ToPcm16(Ching(1.3f, 0.f));
		if (Kind == TEXT("chap")) return ToPcm16(Ching(0.09f, 0.5f));
		if (Kind == TEXT("taphon_hi")) return ToPcm16(Drum(430.f, 360.f, 0.22f, 0.25f));
		if (Kind == TEXT("taphon_lo")) return ToPcm16(Drum(170.f, 128.f, 0.34f, 0.12f));
		if (Kind == TEXT("klong")) return ToPcm16(Drum(74.f, 52.f, 0.9f, 0.3f));
		if (Kind == TEXT("whoosh")) return ToPcm16(Whoosh(0.2f));
		if (Kind == TEXT("hit")) return ToPcm16(Mix(Drum(300.f, 140.f, 0.12f, 0.8f), Ching(0.05f, 0.9f, 0.7f), 0.5f));
		if (Kind == TEXT("clang")) return ToPcm16(Ching(0.35f, 0.3f, 0.6f));
		return ToPcm16({0.f});
	}
}

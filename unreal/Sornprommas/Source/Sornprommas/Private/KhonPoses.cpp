#include "KhonPoses.h"

namespace
{
	constexpr float Pi = UE_PI;

	float EaseOut(float X)
	{
		const float V = FMath::Clamp(X, 0.f, 1.f);
		return 1.f - FMath::Pow(1.f - V, 3.f);
	}

	/** Wind-up from A to B until Split, then strike from B to C over StrikeLen. */
	float Phase(float U, float Split, float A, float B, float C, float StrikeLen = 0.3f)
	{
		if (U < Split)
		{
			return FMath::Lerp(A, B, U / Split);
		}
		return FMath::Lerp(B, C, EaseOut((U - Split) / StrikeLen));
	}

	/** Khon standing posture: knees open and bent, chest lifted, ยืดยุบ sink on the beat. */
	FKhonPose Stance(EKhonKind Kind, float Beat, float T)
	{
		float Crouch = 0.14f, Open = 0.4f;
		if (Kind == EKhonKind::Yak) { Crouch = 0.1f; Open = 0.44f; }
		if (Kind == EKhonKind::Phra) { Crouch = 0.05f; Open = 0.2f; }
		const float Sink = FMath::Square(FMath::Sin(Pi * Beat));

		FKhonPose P;
		P.HipsOffset = FVector(0.f, -Crouch - 0.035f * Sink, 0.f);
		P.Set(TEXT("LegL"), 0.35f, 0.f, -Open);
		P.Set(TEXT("LegR"), 0.35f, 0.f, Open);
		P.Set(TEXT("ShinL"), -0.72f - 0.12f * Sink, 0.f, 0.f);
		P.Set(TEXT("ShinR"), -0.72f - 0.12f * Sink, 0.f, 0.f);
		P.Set(TEXT("Chest"), 0.08f, 0.f, 0.f);
		P.Set(TEXT("Head"), -0.05f, 0.f, 0.f);
		P.Set(TEXT("ArmL"), 0.3f, 0.f, -0.95f);
		P.Set(TEXT("ForeL"), 0.9f, 0.f, -0.9f);
		P.Set(TEXT("ArmR"), 0.45f, 0.f, 0.35f);
		P.Set(TEXT("ForeR"), 1.15f, 0.f, 0.f);
		P.Set(TEXT("Weapon"), -Pi / 2.f, 0.f, 0.f);
		if (Kind == EKhonKind::Ling)
		{
			P.Set(TEXT("Head"), -0.05f, FMath::Sin(T * 2.7f) * 0.22f, FMath::Sin(T * 1.3f) * 0.08f);
		}
		else if (Kind == EKhonKind::Phra)
		{
			P.Set(TEXT("ArmL"), 1.1f, 0.f, -0.25f);
			P.Set(TEXT("ForeL"), 0.35f, 0.f, 0.f);
		}
		return P;
	}

	void Run(FKhonPose& P, EKhonKind Kind, float Mv, float Ph)
	{
		const float S = FMath::Sin(Ph);
		const float C = FMath::Cos(Ph);
		const float Amp = 0.95f * Mv;
		P.Set(TEXT("LegL"), 0.25f + S * Amp, 0.f, -0.14f);
		P.Set(TEXT("LegR"), 0.25f - S * Amp, 0.f, 0.14f);
		P.Set(TEXT("ShinL"), -0.45f - FMath::Max(0.f, -C) * 1.2f * Mv, 0.f, 0.f);
		P.Set(TEXT("ShinR"), -0.45f - FMath::Max(0.f, C) * 1.2f * Mv, 0.f, 0.f);
		P.Set(TEXT("Chest"), -0.3f * Mv, 0.f, 0.f);
		P.Set(TEXT("ArmL"), 0.2f - S * 0.8f * Mv, 0.f, -0.35f);
		P.Set(TEXT("ForeL"), 0.9f, 0.f, 0.f);
		P.Set(TEXT("ArmR"), 0.5f + S * 0.35f * Mv, 0.f, 0.25f);
		const float Hop = Kind == EKhonKind::Ling ? 2.f : 1.f;
		P.HipsOffset = FVector(0.f, -0.08f + FMath::Abs(S) * 0.06f * Hop, 0.f);
	}

	/** ท่าเหาะ / leap: one knee drawn up, free arm flung out. */
	void Air(FKhonPose& P)
	{
		P.Set(TEXT("LegL"), 1.3f, 0.f, -0.2f);
		P.Set(TEXT("ShinL"), -1.7f, 0.f, 0.f);
		P.Set(TEXT("LegR"), 0.1f, 0.f, 0.15f);
		P.Set(TEXT("ShinR"), -0.5f, 0.f, 0.f);
		P.Set(TEXT("ArmL"), 0.4f, 0.f, -1.6f);
		P.Set(TEXT("ForeL"), 0.8f, 0.f, 0.f);
		P.Set(TEXT("Chest"), -0.15f, 0.f, 0.f);
	}

	void ActionPose(FKhonPose& P, const FName& Name, float U)
	{
		const FString N = Name.ToString();
		if (N == TEXT("swing_a"))
		{
			P.Set(TEXT("Chest"), -0.15f, Phase(U, 0.3f, 0.f, -0.9f, 1.05f), 0.f);
			P.Set(TEXT("ArmR"), 1.35f, 0.f, 0.35f);
			P.Set(TEXT("ForeR"), 0.1f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
		}
		else if (N == TEXT("swing_b"))
		{
			P.Set(TEXT("Chest"), -0.15f, Phase(U, 0.3f, 0.f, 0.9f, -1.05f), 0.f);
			P.Set(TEXT("ArmR"), 1.35f, 0.f, -0.1f);
			P.Set(TEXT("ForeR"), 0.1f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
		}
		else if (N == TEXT("slam"))
		{
			P.Set(TEXT("ArmR"), Phase(U, 0.35f, 1.5f, 3.0f, 0.9f, 0.25f), 0.f, 0.15f);
			P.Set(TEXT("ForeR"), 0.05f, 0.f, 0.f);
			P.Set(TEXT("Chest"), U < 0.35f ? 0.3f : -0.5f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
			if (U >= 0.5f)
			{
				P.HipsOffset += FVector(0.f, -0.1f, 0.f);
			}
		}
		else if (N == TEXT("thrust"))
		{
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
			if (U < 0.4f)
			{
				P.Set(TEXT("ArmR"), 0.8f, 0.f, 0.2f);
				P.Set(TEXT("ForeR"), 0.2f, 0.f, 0.f);
				P.Set(TEXT("Chest"), 0.1f, -0.6f, 0.f);
			}
			else
			{
				P.Set(TEXT("ArmR"), 1.57f, 0.f, 0.05f);
				P.Set(TEXT("ForeR"), 0.f, 0.f, 0.f);
				P.Set(TEXT("Chest"), -0.35f, 0.15f, 0.f);
				P.Set(TEXT("LegL"), 0.95f, 0.f, -0.2f);
				P.Set(TEXT("ShinL"), -0.45f, 0.f, 0.f);
				P.Set(TEXT("LegR"), -0.4f, 0.f, 0.2f);
			}
		}
		else if (N == TEXT("plunge"))
		{
			P.Set(TEXT("ArmR"), 0.35f, 0.f, 0.1f);
			P.Set(TEXT("ForeR"), 0.f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
			P.Set(TEXT("Chest"), -0.35f, 0.f, 0.f);
		}
		else if (N == TEXT("special"))
		{
			P.Set(TEXT("ArmL"), 0.2f, 0.f, -2.5f);
			P.Set(TEXT("ArmR"), 0.2f, 0.f, 2.5f);
			P.Set(TEXT("ForeL"), 0.5f, 0.f, 0.f);
			P.Set(TEXT("ForeR"), 0.5f, 0.f, 0.f);
			P.Set(TEXT("Chest"), 0.3f, 0.f, 0.f);
			P.Set(TEXT("Head"), 0.35f, 0.f, 0.f);
		}
		else if (N == TEXT("hurt"))
		{
			P.Set(TEXT("Chest"), 0.45f, 0.f, 0.f);
			P.Set(TEXT("Head"), 0.3f, 0.f, 0.f);
			P.Set(TEXT("ArmL"), 0.6f, 0.f, -1.3f);
		}
		else if (N == TEXT("windup"))
		{
			P.Set(TEXT("ArmR"), 2.9f, 0.f, 0.3f);
			P.Set(TEXT("ForeR"), 0.4f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi * 0.9f, 0.f, 0.f);
			P.Set(TEXT("Chest"), 0.25f, -0.3f, 0.f);
		}
		else if (N == TEXT("strike"))
		{
			P.Set(TEXT("ArmR"), Phase(U, 0.05f, 2.9f, 2.9f, 0.7f, 0.4f), 0.f, 0.2f);
			P.Set(TEXT("ForeR"), 0.1f, 0.f, 0.f);
			P.Set(TEXT("Weapon"), -Pi, 0.f, 0.f);
			P.Set(TEXT("Chest"), Phase(U, 0.05f, 0.25f, 0.25f, -0.45f, 0.4f), Phase(U, 0.05f, -0.3f, -0.3f, 0.3f, 0.4f), 0.f);
		}
		else if (N == TEXT("bow_draw"))
		{
			P.Set(TEXT("ArmL"), 1.5f, 0.f, -0.1f);
			P.Set(TEXT("ForeL"), 0.f, 0.f, 0.f);
			P.Set(TEXT("ArmR"), 1.45f, 0.f, 0.3f);
			P.Set(TEXT("ForeR"), 1.8f * FMath::Clamp(U * 1.5f, 0.f, 1.f), 0.f, 0.f);
			P.Set(TEXT("Chest"), 0.05f, -0.3f, 0.f);
		}
		else if (N == TEXT("bow_release"))
		{
			P.Set(TEXT("ArmL"), 1.5f, 0.f, -0.1f);
			P.Set(TEXT("ArmR"), 1.2f, 0.f, 0.9f);
			P.Set(TEXT("ForeR"), 0.2f, 0.f, 0.f);
		}
		else if (N == TEXT("victory"))
		{
			P.Set(TEXT("ArmL"), 0.3f, 0.f, -2.3f);
			P.Set(TEXT("ArmR"), 0.6f, 0.f, 1.2f);
			P.Set(TEXT("ForeL"), 0.9f, 0.f, 0.f);
			P.Set(TEXT("Chest"), 0.2f, 0.25f, 0.f);
		}
	}
}

namespace KhonPoses
{
	FKhonPose Compute(const FKhonPoseContext& C)
	{
		FKhonPose P = Stance(C.Kind, C.Beat, C.Time);
		if (!C.bGrounded)
		{
			Air(P);
		}
		else if (C.Move > 0.05f)
		{
			Run(P, C.Kind, C.Move, C.RunPhase);
		}
		if (!C.Action.IsNone())
		{
			ActionPose(P, C.Action, C.U);
		}
		return P;
	}
}

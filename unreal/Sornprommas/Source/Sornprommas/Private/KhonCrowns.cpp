#include "KhonCrowns.h"

#include "KhonParts.h"
#include "SornTypes.h"

using namespace KhonParts;

namespace
{
	void Face(AActor* O, USceneComponent* Head, const FKhonSpec& Spec, float R)
	{
		UKhonMaterialSubsystem* L = Lib(O);
		UMaterialInterface* GoldM = Gold(O);
		UMaterialInterface* White = L->Mat(Sorn::Col::EyeWhite, 0.f, 0.3f);
		UMaterialInterface* Pupil = L->Mat(Sorn::Col::Pupil, 0.f, 0.2f);
		UMaterialInterface* FangM = L->Mat(Sorn::Col::Fang, 0.1f, 0.2f);
		const float Z = -R + 0.015f;

		float EyeR = 0.04f, EyeX = 0.075f;
		if (Spec.Kind == EKhonKind::Yak) { EyeR = 0.055f; EyeX = 0.085f; }
		if (Spec.Kind == EKhonKind::Phra) { EyeR = 0.028f; EyeX = 0.06f; }
		for (const float Side : {-1.f, 1.f})
		{
			if (Spec.Kind == EKhonKind::Phra)
			{
				Part(O, Head, EKhonShape::Sphere, Pupil, FVector(EyeX * Side, 0.06f, Z + 0.01f), FVector(EyeR * 3.f, EyeR * 0.9f, EyeR * 1.2f));
			}
			else
			{
				Part(O, Head, EKhonShape::Sphere, White, FVector(EyeX * Side, 0.07f, Z + 0.01f), FVector(EyeR * 2.f));
				Part(O, Head, EKhonShape::Sphere, Pupil, FVector(EyeX * Side, 0.07f, Z - EyeR * 0.7f), FVector(EyeR));
			}
		}
		if (Spec.Kind != EKhonKind::Phra)
		{
			Part(O, Head, EKhonShape::Cube, GoldM, FVector(0, 0.12f, Z + 0.02f), FVector(0.28, 0.025, 0.04));
		}

		switch (Spec.Mouth)
		{
		case EKhonMouth::Open:
			Part(O, Head, EKhonShape::Cube, L->Mat(Sorn::Col::MouthRed, 0.f, 0.5f), FVector(0, -0.07f, Z + 0.02f), FVector(0.13, 0.07, 0.06));
			for (const float Side : {-1.f, 1.f})
			{
				Part(O, Head, EKhonShape::Cone, FangM, FVector(0.045f * Side, -0.05f, Z - 0.005f), FVector(0.024, 0.05, 0.024), FVector(180, 0, 0));
				Part(O, Head, EKhonShape::Cone, FangM, FVector(0.045f * Side, -0.095f, Z - 0.005f), FVector(0.024, 0.05, 0.024));
			}
			break;
		case EKhonMouth::Grin:
			Part(O, Head, EKhonShape::Cube, L->Mat(Sorn::Col::KhonRedDark, 0.f, 0.5f), FVector(0, -0.075f, Z + 0.02f), FVector(0.2, 0.045, 0.05));
			for (const float Side : {-1.f, 1.f})
			{
				Part(O, Head, EKhonShape::Cone, FangM, FVector(0.08f * Side, -0.04f, Z - 0.005f), FVector(0.036, 0.09, 0.036));
			}
			break;
		default:
			Part(O, Head, EKhonShape::Cube, L->Mat(Sorn::Col::MouthRed, 0.f, 0.5f), FVector(0, -0.06f, Z + 0.02f), FVector(0.06, 0.018, 0.03));
			break;
		}

		if (Spec.Kind == EKhonKind::Ling)
		{
			for (const float Side : {-1.f, 1.f})
			{
				Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0.19f * Side, -0.02f, 0), FVector(0.08, 0.02, 0.08), FVector(0, 0, 90));
			}
		}
	}

	void CrownHanuman(AActor* O, USceneComponent* Head)
	{
		UMaterialInterface* GoldM = Gold(O);
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.14f, 0), FVector(0.4, 0.07, 0.4), FVector(-8, 0, 0));
		Part(O, Head, EKhonShape::Cone, GoldM, FVector(0, 0.23f, -0.17f), FVector(0.12, 0.14, 0.03));
		for (const float Side : {-1.f, 1.f})
		{
			Part(O, Head, EKhonShape::Cone, GoldM, FVector(0.12f * Side, 0.21f, -0.13f), FVector(0.08, 0.1, 0.03), FVector(0, -35.f * Side, 0));
		}
	}

	void CrownYaksha(AActor* O, USceneComponent* Head, const FLinearColor& Band)
	{
		UMaterialInterface* GoldM = Gold(O);
		UMaterialInterface* BandM = Lib(O)->Mat(Band, 0.2f, 0.4f);
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.15f, 0), FVector(0.4, 0.09, 0.4));
		Part(O, Head, EKhonShape::Cylinder, BandM, FVector(0, 0.2f, 0), FVector(0.38, 0.03, 0.38));
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.28f, 0), FVector(0.3, 0.14, 0.3));
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.41f, 0), FVector(0.2, 0.14, 0.2));
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.53f, 0), FVector(0.125, 0.12, 0.125));
		Part(O, Head, EKhonShape::Cone, GoldM, FVector(0, 0.76f, 0), FVector(0.09, 0.34, 0.09));
		for (const float Side : {-1.f, 1.f})
		{
			Part(O, Head, EKhonShape::Cone, GoldM, FVector(0.2f * Side, 0.02f, 0.02f), FVector(0.03, 0.18, 0.1), FVector(0, 0, -20.f * Side));
		}
	}

	void CrownPhra(AActor* O, USceneComponent* Head)
	{
		UMaterialInterface* GoldM = Gold(O);
		Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.14f, 0), FVector(0.34, 0.07, 0.34));
		for (int32 I = 0; I < 4; ++I)
		{
			const float D = 2.f * (0.14f - I * 0.022f);
			Part(O, Head, EKhonShape::Cylinder, GoldM, FVector(0, 0.2f + I * 0.07f, 0), FVector(D, 0.06, D));
		}
		Part(O, Head, EKhonShape::Cone, GoldM, FVector(0, 0.69f, 0), FVector(0.1, 0.42, 0.1));
		Part(O, Head, EKhonShape::Sphere, Lib(O)->Mat(Sorn::Col::UiSignal, 0.3f, 0.2f, 2.f), FVector(0, 0.46f, -0.06f), FVector(0.04));
		for (const float Side : {-1.f, 1.f})
		{
			Part(O, Head, EKhonShape::Cone, GoldM, FVector(0.17f * Side, -0.02f, 0.06f), FVector(0.02, 0.22, 0.06), FVector(20, 0, 25.f * Side));
		}
	}
}

namespace KhonCrowns
{
	void BuildHead(AActor* Owner, USceneComponent* Head, const FKhonSpec& Spec)
	{
		const float R = Spec.Kind == EKhonKind::Phra ? 0.165f : 0.19f;
		UMaterialInterface* SkinM = Lib(Owner)->Mat(Spec.Skin, 0.f, 0.45f);
		Part(Owner, Head, EKhonShape::Sphere, SkinM, FVector(0, 0.04f, 0), FVector(2.f * R, 2.1f * R, 1.96f * R));
		Face(Owner, Head, Spec, R);
		switch (Spec.Crown)
		{
		case EKhonCrown::Hanuman: CrownHanuman(Owner, Head); break;
		case EKhonCrown::Yaksha: CrownYaksha(Owner, Head, Spec.Trim); break;
		case EKhonCrown::Phra: CrownPhra(Owner, Head); break;
		default: break;
		}
	}
}

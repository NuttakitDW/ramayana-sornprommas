#include "BattleDirector.h"

#include "EngineUtils.h"
#include "ErawanBoss.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HanumanCharacter.h"
#include "KhonFigureComponent.h"
#include "MusicDirector.h"
#include "SornFx.h"
#include "SornGameMode.h"
#include "SornHUD.h"
#include "SornTypes.h"
#include "StoryText.h"
#include "YakshaEnemy.h"

ABattleDirector::ABattleDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

AHanumanCharacter* ABattleDirector::Hero() const
{
	const ASornGameMode* GM = ASornGameMode::Get(this);
	return GM ? GM->Hanuman() : nullptr;
}

ASornHUD* ABattleDirector::Hud() const
{
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	return PC ? Cast<ASornHUD>(PC->GetHUD()) : nullptr;
}

void ABattleDirector::Cue(const FString& Id)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
	{
		GM->Music->PlayCue(Id);
	}
}

void ABattleDirector::Begin(const FString& StartBeat)
{
	const ASornGameMode* GM = ASornGameMode::Get(this);
	bQuick = GM && GM->bQuick;
	bAutoConfirm = GM && (GM->bQuick || GM->bAutobot);
	if (AHanumanCharacter* H = Hero())
	{
		H->OnDied.AddUObject(this, &ABattleDirector::OnPlayerDied);
	}
	SpawnAllies();
	StartTime = FPlatformTime::Seconds();

	const TArray<FString> Order = {TEXT("intro"), TEXT("wave1"), TEXT("wave2"), TEXT("boss")};
	const int32 From = FMath::Max(Order.IndexOfByKey(StartBeat), 0);
	if (From <= 0) QueueIntro();
	if (From <= 1) QueueWave1();
	if (From <= 2) QueueWave2();
	QueueBossIntro();
	QueueBossFight();
	QueueFinale();
	Advance();
}

// ------------------------------------------------------------------ step machine

void ABattleDirector::Step(TFunction<void()> Run, EWait InWait, float Seconds)
{
	Steps.Add({MoveTemp(Run), InWait, Seconds});
}

void ABattleDirector::Tween(float Duration, TFunction<void(float)> Apply, TFunction<void()> Done)
{
	Tweens.Add({MoveTemp(Apply), MoveTemp(Done), 0.f, FMath::Max(Duration, 0.01f)});
}

void ABattleDirector::Advance()
{
	while (!bOver && Steps.IsValidIndex(StepIndex + 1))
	{
		++StepIndex;
		FStep& S = Steps[StepIndex];
		bConfirmed = false;
		WaitLeft = S.Seconds * (bQuick ? 0.4f : 1.f);
		if (S.Wait == EWait::Confirm && bAutoConfirm)
		{
			S.Wait = EWait::Time;
			WaitLeft = 1.5f;
		}
		S.Run();
		if (S.Wait != EWait::None)
		{
			return;
		}
	}
}

bool ABattleDirector::WaitSatisfied() const
{
	const FStep& S = Steps[StepIndex];
	switch (S.Wait)
	{
	case EWait::Time: return WaitLeft <= 0.f;
	case EWait::Confirm: return bConfirmed;
	case EWait::WaveClear: return Alive <= 0 && WaitLeft <= 0.f;
	case EWait::BossDefeated: return bBossDefeated;
	default: return true;
	}
}

void ABattleDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float RealDt = GetWorld()->DeltaRealTimeSeconds;
	for (int32 I = Tweens.Num() - 1; I >= 0; --I)
	{
		FTween& Tw = Tweens[I];
		Tw.T += RealDt;
		const float U = FMath::Clamp(Tw.T / Tw.Duration, 0.f, 1.f);
		Tw.Apply(U);
		if (U >= 1.f)
		{
			TFunction<void()> Done = MoveTemp(Tw.Done);
			Tweens.RemoveAt(I);
			if (Done)
			{
				Done();
			}
		}
	}
	if (bOver || !Steps.IsValidIndex(StepIndex))
	{
		return;
	}
	const EWait W = Steps[StepIndex].Wait;
	if (W == EWait::Time || (W == EWait::WaveClear && Alive <= 0))
	{
		WaitLeft -= RealDt;
	}
	if (WaitSatisfied())
	{
		Advance();
	}
}

void ABattleDirector::OnConfirm()
{
	bConfirmed = true;
}

// ------------------------------------------------------------------ beats

void ABattleDirector::QueueIntro()
{
	Step([this]
	{
		Phase = TEXT("intro");
		Hero()->bInputEnabled = false;
		Cue(TEXT("wa"));
		Hud()->ShowCard(StoryText::Title(), true);
	}, EWait::Confirm);
	Step([this]
	{
		Hud()->HideCard();
		Cue(TEXT("kraw_nok"));
		Hud()->ShowBanner(StoryText::March(), 3.5f);
	}, EWait::Time, 3.f);
}

void ABattleDirector::QueueWave1()
{
	Step([this]
	{
		Phase = TEXT("wave1");
		StartTime = FPlatformTime::Seconds();
		Hero()->bInputEnabled = true;
		Cue(TEXT("choet"));
		ObjectiveTh = TEXT("ปราบพลยักษ์ขัดตาทัพ");
		ObjectiveEn = TEXT("DEFEAT THE DEMON VANGUARD");
		StartWave(4);
		for (int32 I = 0; I < 4; ++I)
		{
			SpawnSoldier(false);
		}
	}, EWait::WaveClear, 0.8f);
}

void ABattleDirector::QueueWave2()
{
	Step([this]
	{
		Phase = TEXT("wave2");
		Hero()->bInputEnabled = true;
		MaxTokens = 3;
		Cue(TEXT("choet_klong"));
		Hud()->ShowBanner(StoryText::Kampan(), 6.f);
		ObjectiveTh = TEXT("ปราบกำปั่นและทัพยักษ์");
		ObjectiveEn = TEXT("DEFEAT KAMPAN AND HIS DEMONS");
		StartWave(5);
		SpawnSoldier(true);
		for (int32 I = 0; I < 4; ++I)
		{
			SpawnSoldier(false);
		}
	}, EWait::WaveClear, 0.8f);
}

void ABattleDirector::QueueBossIntro()
{
	Step([this]
	{
		Phase = TEXT("boss_intro");
		Hero()->EnterCutscene();
		Hud()->SetObjective(TEXT(""), TEXT(""));
	}, EWait::Time, 0.8f);
	Step([this]
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Boss = GetWorld()->SpawnActor<AErawanBoss>(AErawanBoss::StaticClass(), FTransform::Identity, P);
		Boss->Appear();
		Boss->OnHeadBroken.AddUObject(this, &ABattleDirector::OnHeadBroken);
		Boss->OnDefeated.AddLambda([this] { bBossDefeated = true; });
		Hero()->CameraFocus = Boss;
		Cue(TEXT("choet_ching"));
		Hud()->ShowBanner(StoryText::ErawanAppears(), 4.f);
	}, EWait::Time, 3.6f);
	Step([this]
	{
		Hud()->ShowBanner(StoryText::ArrowLoosed(), 4.f);
		Boss->Rider()->Play(TEXT("bow_draw"), 1.2f, true);
	}, EWait::Time, 2.4f);
	Step([this]
	{
		LooseBrahmastra();
		Hud()->ShowBanner(StoryText::ArmyFalls(), 4.5f);
	}, EWait::Time, 4.f);
	Step([this]
	{
		Cue(TEXT("ling_lot"));
		Hero()->FacePoint(Boss->GetActorLocation());
		Hud()->ShowBanner(StoryText::LingLot(), 6.f);
	}, EWait::Time, 3.5f);
	Step([this] { Hero()->LeaveCutscene(); });
}

void ABattleDirector::QueueBossFight()
{
	Step([this]
	{
		Phase = TEXT("boss");
		MaxTokens = 2;
		Hud()->ShowBoss(Boss);
		Boss->bActive = true;
		Cue(TEXT("choet_klong"));
		OnHeadBroken(-1, Boss->HeadsLeft());
	}, EWait::BossDefeated);
}

void ABattleDirector::QueueFinale()
{
	Step([this]
	{
		Phase = TEXT("finale");
		Hero()->EnterCutscene();
		if (ASornGameMode* GM = ASornGameMode::Get(this)) GM->Fx->SlowMo = 0.35f;
		KillAllSoldiers();
		Hud()->HideBoss();
		Hud()->SetObjective(TEXT(""), TEXT(""));
		Hud()->ShowBanner(StoryText::NeckBreak(), 5.f);
	}, EWait::Time, 1.4f);
	Step([this]
	{
		// Hanuman leaps onto Erawan to seize the bow.
		ASornGameMode::Get(this)->Fx->SlowMo = 1.f;
		AHanumanCharacter* H = Hero();
		H->SetPuppet(true);
		H->FacePoint(Boss->GetActorLocation());
		const FVector Start = H->GetActorLocation();
		const FVector Top = Boss->Rider()->GetComponentLocation() + FVector(-110.f, 0, 110.f);
		const FVector Mid = (Start + Top) * 0.5f + FVector(0, 0, 400.f);
		Tween(0.8f, [H, Start, Mid, Top](float U)
		{
			const FVector A = FMath::Lerp(Start, Mid, U);
			const FVector B = FMath::Lerp(Mid, Top, U);
			H->SetActorLocation(FMath::Lerp(A, B, U));
		}, [H] { H->Figure->Play(TEXT("slam"), 0.5f); });
	}, EWait::Time, 1.3f);
	Step([this]
	{
		// Indrajit swings the bow and strikes Hanuman down.
		Boss->Rider()->Play(TEXT("bow_release"), 0.4f);
		Cue(TEXT("ot_haep"));
		Hud()->HurtFlash();
		AHanumanCharacter* H = Hero();
		H->Shake(0.8f);
		ASornGameMode::Get(this)->Fx->Sparks(H->GetActorLocation() + FVector(0, 0, 60), FLinearColor::White, 40, 900.f);
		Hud()->ShowBanner(StoryText::StruckDown(), 6.f);
		const FVector Start = H->GetActorLocation();
		const FVector Land(Start.X * 0.6f - 300.f, Start.Y * 0.6f, 92.f);
		Tween(0.9f, [H, Start, Land](float U) { H->SetActorLocation(FMath::Lerp(Start, Land, U * U)); }, [this, H, Land]
		{
			H->SetPuppet(false);
			H->KnockDown();
			ASornGameMode* GM = ASornGameMode::Get(this);
			GM->Fx->Ring(Land - FVector(0, 0, 90), 400.f, Sorn::Col::Gold);
			GM->Music->Sfx(TEXT("klong"), 0.6f, 1.4f);
			H->Shake(0.6f);
			// The illusion lifts away into the sky.
			H->CameraFocus = nullptr;
			const FVector BStart = Boss->GetActorLocation();
			Tween(3.f, [this, BStart](float U)
			{
				if (Boss)
				{
					Boss->SetActorLocation(BStart + FVector(3000.f, 0, 3000.f) * U);
					Boss->SetActorScale3D(FVector(FMath::Lerp(1.f, 0.2f, U)));
				}
			}, [this] { if (Boss) Boss->Destroy(); });
		});
	}, EWait::Time, 4.4f);
	Step([this] { Hud()->FadeTo(0.72f, 1.5f); }, EWait::Time, 2.f);
	Step([this]
	{
		Hud()->ShowCard(StoryText::Wind(), false);
		WindGust();
	}, EWait::Time, 4.5f);
	Step([this]
	{
		Hud()->HideCard();
		Hud()->FadeTo(0.f, 1.2f);
		Hero()->Revive();
		Cue(TEXT("khaek_bora_thet"));
	}, EWait::Time, 1.2f);
	Step([this] { Hero()->Figure->Play(TEXT("victory"), 3.f, true); }, EWait::Time, 2.5f);
	Step([this]
	{
		Phase = TEXT("end");
		Hud()->ShowCard(StoryText::EndCard(Stats()), false);
	});
}

// ------------------------------------------------------------------ set pieces

void ABattleDirector::SpawnAllies()
{
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Allies = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, P);
	USceneComponent* Root = NewObject<USceneComponent>(Allies, TEXT("AlliesRoot"));
	Allies->SetRootComponent(Root);
	Root->RegisterComponent();
	Allies->SetActorLocation(Sorn::G(0, 0, 17.f));

	auto Add = [this, Root](const FKhonSpec& Spec, const FVector& PosM)
	{
		UKhonFigureComponent* F = NewObject<UKhonFigureComponent>(Allies);
		F->SetupAttachment(Root);
		F->RegisterComponent();
		F->SetRelativeLocation(Sorn::G(PosM.X, PosM.Y, PosM.Z));
		F->Build(Spec);
		AllyFigures.Add(F);
	};
	// พระลักษมณ์: gold, unmasked, with bow.
	FKhonSpec Lakshman;
	Lakshman.Kind = EKhonKind::Phra;
	Lakshman.Skin = Sorn::Col::LakshmanGold;
	Lakshman.Cloth = Sorn::Col::KhonRed;
	Lakshman.Trim = Sorn::Col::UiDepth;
	Lakshman.Crown = EKhonCrown::Phra;
	Lakshman.Weapon = EKhonWeapon::Bow;
	Add(Lakshman, FVector(0, 0, 1.f));
	// พลวานร: monkey soldiers in many mask colours.
	const FLinearColor Skins[] = {
		FLinearColor(0.62f, 0.14f, 0.10f), FLinearColor(0.16f, 0.45f, 0.22f), FLinearColor(0.12f, 0.12f, 0.15f),
		FLinearColor(0.15f, 0.26f, 0.56f), FLinearColor(0.70f, 0.52f, 0.18f), FLinearColor(0.55f, 0.20f, 0.30f)};
	for (int32 I = 0; I < UE_ARRAY_COUNT(Skins); ++I)
	{
		FKhonSpec M;
		M.Kind = EKhonKind::Ling;
		M.Skin = Skins[I];
		M.Cloth = Sorn::Col::ClothDark;
		M.Trim = Sorn::Col::KhonRed;
		M.Crown = EKhonCrown::Hanuman;
		M.Weapon = EKhonWeapon::Club;
		M.Mouth = EKhonMouth::Open;
		M.bTail = true;
		const float Side = I % 2 == 0 ? -1.f : 1.f;
		const float Slot = I / 2 + 1;
		Add(M, FVector(Side * Slot * 1.9f, 0, Slot * 0.7f));
	}
}

void ABattleDirector::AlliesFall()
{
	for (int32 I = 0; I < AllyFigures.Num(); ++I)
	{
		TWeakObjectPtr<UKhonFigureComponent> F = AllyFigures[I];
		Tween(0.08f * I + 0.01f, [](float) {}, [F]
		{
			if (F.IsValid())
			{
				F->Flash(FLinearColor(1.f, 0.85f, 0.5f), 0.3f);
				F->bDown = true;
			}
		});
	}
}

void ABattleDirector::LooseBrahmastra()
{
	ASornGameMode* GM = ASornGameMode::Get(this);
	const FVector From = Boss->Rider()->GetComponentLocation() + FVector(0, 0, 140.f);
	const FVector To = Allies->GetActorLocation() + FVector(0, 0, 100.f);
	GM->Fx->Beam(From, To, FLinearColor(1.f, 0.85f, 0.5f), 0.6f);
	GM->Fx->Ring(FVector(To.X, To.Y, 0.f), 1400.f, FLinearColor(1.f, 0.8f, 0.4f), 0.9f);
	GM->Fx->Sparks(To, FLinearColor(1.f, 0.85f, 0.5f), 60, 1200.f);
	GM->Music->Sfx(TEXT("klong"), 0.4f, 1.6f);
	Hero()->Shake(0.7f);
	AlliesFall();
}

void ABattleDirector::WindGust()
{
	for (int32 I = 0; I < 6; ++I)
	{
		Tween(I * 0.5f + 0.01f, [](float) {}, [this]
		{
			AHanumanCharacter* H = Hero();
			ASornGameMode* GM = ASornGameMode::Get(this);
			if (!H || !GM)
			{
				return;
			}
			const FVector Feet = H->GetActorLocation() - FVector(0, 0, 90.f);
			GM->Fx->Sparks(Feet + FVector(FMath::FRandRange(-100.f, 100.f), FMath::FRandRange(-100.f, 100.f), 50.f), FLinearColor(0.75f, 0.88f, 1.f), 24, 500.f);
			GM->Fx->Ring(Feet, 350.f, FLinearColor(0.75f, 0.88f, 1.f), 0.8f);
		});
	}
}

void ABattleDirector::KillAllSoldiers()
{
	for (TActorIterator<AYakshaEnemy> It(GetWorld()); It; ++It)
	{
		if (It->IsAlive())
		{
			It->ReceiveHit(9999.f, Hero()->GetActorLocation(), 600.f, 0);
		}
	}
}

// ------------------------------------------------------------------ waves

void ABattleDirector::StartWave(int32 Total)
{
	WaveTotal = Total;
	WaveKilled = 0;
	UpdateObjective();
}

AYakshaEnemy* ABattleDirector::SpawnSoldier(bool bCaptain)
{
	const float A = FMath::FRandRange(-UE_PI * 0.85f, -UE_PI * 0.15f);
	const FVector Pos = Sorn::G(FMath::Cos(A) * 23.f, 1.f, FMath::Sin(A) * 23.f);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AYakshaEnemy* Y = GetWorld()->SpawnActorDeferred<AYakshaEnemy>(AYakshaEnemy::StaticClass(), FTransform(Pos), nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Y->Configure(bCaptain ? Sorn::Col::KhonRed : Sorn::Col::YakshaSkin(FMath::RandRange(0, 4)), bCaptain, bCaptain ? TEXT("KAMPAN") : TEXT(""));
	Y->FinishSpawning(FTransform(Pos));
	Y->OnDied.AddUObject(this, &ABattleDirector::OnEnemyDied);
	++Alive;
	return Y;
}

void ABattleDirector::OnEnemyDied(AYakshaEnemy* /*Enemy*/)
{
	--Alive;
	++WaveKilled;
	UpdateObjective();
}

void ABattleDirector::UpdateObjective()
{
	if (WaveTotal > 0 && Phase.StartsWith(TEXT("wave")))
	{
		Hud()->SetObjective(ObjectiveTh, FString::Printf(TEXT("%s   %d/%d"), *ObjectiveEn, WaveKilled, WaveTotal));
	}
}

void ABattleDirector::OnHeadBroken(int32 Index, int32 Remaining)
{
	Hud()->SetObjective(TEXT("หักคอเอราวัณ  ฟาดเศียรช้างขณะคุกเข่า"),
		FString::Printf(TEXT("BREAK ERAWAN'S NECKS: STRIKE THE HEADS WHILE IT KNEELS   %d/3"), 3 - Remaining));
	if (Index < 0)
	{
		return;
	}
	if (Remaining == 2)
	{
		Hud()->ShowBanner(StoryText::NeckBreak(), 4.f);
	}
	if (Remaining > 0)
	{
		SpawnSoldier(false);
		SpawnSoldier(false);
	}
}

// ------------------------------------------------------------------ tokens & flow

bool ABattleDirector::RequestToken(AActor* Enemy)
{
	Tokens.RemoveAll([](const TWeakObjectPtr<AActor>& W) { return !W.IsValid(); });
	if (Tokens.Contains(Enemy))
	{
		return true;
	}
	if (Tokens.Num() < MaxTokens)
	{
		Tokens.Add(Enemy);
		return true;
	}
	return false;
}

void ABattleDirector::ReleaseToken(AActor* Enemy)
{
	Tokens.Remove(Enemy);
}

void ABattleDirector::OnPlayerDied()
{
	if (bOver)
	{
		return;
	}
	bOver = true;
	Phase = TEXT("game_over");
	if (Boss)
	{
		Boss->bActive = false;
	}
	Cue(TEXT("kraw_ram_phama"));
	Hud()->HideBoss();
	Tween(1.5f, [](float) {}, [this] { Hud()->ShowCard(StoryText::GameOver(), false); });
}

FString ABattleDirector::Stats() const
{
	const int32 Secs = static_cast<int32>(FPlatformTime::Seconds() - StartTime);
	const AHanumanCharacter* H = Hero();
	return FString::Printf(TEXT("TIME %02d:%02d    MAX COMBO %d    HITS TAKEN %d"), Secs / 60, Secs % 60,
		H ? H->MaxCombo : 0, H ? H->HitsTaken : 0);
}

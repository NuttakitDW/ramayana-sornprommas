#include "SornGameMode.h"

#include "ArenaBuilder.h"
#include "BattleDirector.h"
#include "EngineUtils.h"
#include "ErawanBoss.h"
#include "HanumanCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "MusicDirector.h"
#include "SornFx.h"
#include "SornHUD.h"
#include "SornTypes.h"
#include "UnrealClient.h"

ASornGameMode::ASornGameMode()
{
	DefaultPawnClass = AHanumanCharacter::StaticClass();
	HUDClass = ASornHUD::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

ASornGameMode* ASornGameMode::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetAuthGameMode<ASornGameMode>() : nullptr;
}

void ASornGameMode::StartPlay()
{
	const TCHAR* Cmd = FCommandLine::Get();
	bAutobot = FParse::Param(Cmd, TEXT("autobot"));
	bQuick = FParse::Param(Cmd, TEXT("quick"));
	FParse::Value(Cmd, TEXT("start="), StartBeat);
	FString Shots;
	if (FParse::Value(Cmd, TEXT("shots="), Shots, false))
	{
		TArray<FString> Parts;
		Shots.ParseIntoArray(Parts, TEXT(","));
		for (const FString& P : Parts)
		{
			ShotTimes.Add(FCString::Atof(*P));
		}
	}
	FParse::Value(Cmd, TEXT("quitat="), QuitAt);

	UWorld* World = GetWorld();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AArenaBuilder>(AArenaBuilder::StaticClass(), FTransform::Identity, Params);
	Music = World->SpawnActor<AMusicDirector>(AMusicDirector::StaticClass(), FTransform::Identity, Params);
	Fx = World->SpawnActor<ASornFx>(ASornFx::StaticClass(), FTransform::Identity, Params);

	Super::StartPlay(); // spawns Hanuman via RestartPlayer

	Director = World->SpawnActor<ABattleDirector>(ABattleDirector::StaticClass(), FTransform::Identity, Params);
	Director->Begin(StartBeat);
}

void ASornGameMode::RestartPlayer(AController* NewPlayer)
{
	// No PlayerStart in the level: Hanuman starts south of centre facing Lanka (+X).
	RestartPlayerAtTransform(NewPlayer, FTransform(FRotator::ZeroRotator, Sorn::G(0.f, 1.f, 8.f)));
}

AHanumanCharacter* ASornGameMode::Hanuman() const
{
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	return PC ? Cast<AHanumanCharacter>(PC->GetPawn()) : nullptr;
}

void ASornGameMode::RegisterHittable(AActor* Actor)
{
	Hittables.AddUnique(Actor);
}

void ASornGameMode::UnregisterHittable(AActor* Actor)
{
	Hittables.Remove(Actor);
}

void ASornGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Hittables.RemoveAll([](const TWeakObjectPtr<AActor>& W) { return !W.IsValid(); });
	if (ShotTimes.Num() > 0 || QuitAt > 0.f)
	{
		TickHarness();
	}
}

void ASornGameMode::TickHarness()
{
	RealElapsed += GetWorld()->DeltaRealTimeSeconds;
	if (RealElapsed >= NextLog)
	{
		NextLog += 3.f;
		const AHanumanCharacter* H = Hanuman();
		FString BossTxt = TEXT("-");
		for (TActorIterator<AErawanBoss> It(GetWorld()); It; ++It)
		{
			BossTxt = FString::Printf(TEXT("%s heads=%.0f/%.0f/%.0f"), *It->State.ToString(),
				It->HeadHp[0], It->HeadHp[1], It->HeadHp[2]);
		}
		UE_LOG(LogTemp, Display, TEXT("[harness] t=%.1f phase=%s hp=%.0f power=%.0f state=%s hittables=%d boss=%s cue=%s"),
			RealElapsed, Director ? *Director->Phase : TEXT("?"), H ? H->Hp : -1.f, H ? H->Power : -1.f,
			H ? *H->State.ToString() : TEXT("?"), Hittables.Num(), *BossTxt, Music ? *Music->Current : TEXT("?"));
	}
	if (ShotsTaken < ShotTimes.Num() && RealElapsed >= ShotTimes[ShotsTaken])
	{
		const FString Name = FString::Printf(TEXT("%s/Screenshots/shot_%02d_%03ds.png"), *FPaths::ProjectSavedDir(),
			ShotsTaken + 1, FMath::RoundToInt(ShotTimes[ShotsTaken]));
		FScreenshotRequest::RequestScreenshot(Name, true, false);
		UE_LOG(LogTemp, Display, TEXT("[harness] screenshot %s"), *Name);
		++ShotsTaken;
	}
	if (QuitAt > 0.f && RealElapsed >= QuitAt)
	{
		UE_LOG(LogTemp, Display, TEXT("[harness] quit at %.1f"), RealElapsed);
		QuitAt = -1.f;
		FGenericPlatformMisc::RequestExit(false);
	}
}

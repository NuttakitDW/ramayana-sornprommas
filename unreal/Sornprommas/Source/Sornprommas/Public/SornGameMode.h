#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SornGameMode.generated.h"

class AMusicDirector;
class ASornFx;
class ABattleDirector;
class AHanumanCharacter;

/**
 * Builds the battlefield and owns the shared systems (music, fx, director).
 *
 * Command-line switches (for automated play-testing):
 *   -autobot            Hanuman plays himself
 *   -quick              shorter story beats, auto-confirm cards
 *   -start=wave2|boss   jump to a later beat
 *   -shots=5,12,30      take screenshots at these real-time seconds
 *   -quitat=40          quit after N seconds
 */
UCLASS()
class SORNPROMMAS_API ASornGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASornGameMode();

	static ASornGameMode* Get(const UObject* WorldContext);

	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void Tick(float DeltaSeconds) override;

	AHanumanCharacter* Hanuman() const;

	void RegisterHittable(AActor* Actor);
	void UnregisterHittable(AActor* Actor);

	UPROPERTY() TObjectPtr<AMusicDirector> Music;
	UPROPERTY() TObjectPtr<ASornFx> Fx;
	UPROPERTY() TObjectPtr<ABattleDirector> Director;

	TArray<TWeakObjectPtr<AActor>> Hittables;

	bool bAutobot = false;
	bool bQuick = false;
	/** Dev: camera faces Hanuman's mask (-portrait) for checking head assets. */
	bool bPortrait = false;
	FString StartBeat = TEXT("intro");

private:
	void TickHarness();

	TArray<float> ShotTimes;
	float QuitAt = -1.f;
	float RealElapsed = 0.f;
	float NextLog = 0.f;
	int32 ShotsTaken = 0;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDirector.generated.h"

class AErawanBoss;
class AYakshaEnemy;
class AHanumanCharacter;
class ASornHUD;
class UKhonFigureComponent;

/**
 * Runs the demo's story beats (after lore/story/02-synopsis.md):
 *   title -> march (กราวนอก) -> vanguard wave (เชิด) -> Kampan's wave (เชิดกลอง)
 *   -> Erawan appears, the Brahmastra fells the army (เชิดฉิ่ง / ร่ายรุด)
 *   -> Hanuman rages (ลิงโลด) -> boss -> neck broken, bow seized, Hanuman struck
 *   down (ลิงลาน / โอดแหบ) -> revived by the wind (แขกบรเทศ) -> end card.
 * Per Khon convention the show never ends on the fallen tableau (พระล้ม).
 *
 * Implemented as a queue of steps; each step runs, then waits for time,
 * a confirm press, a cleared wave or the boss's defeat.
 */
UCLASS()
class SORNPROMMAS_API ABattleDirector : public AActor
{
	GENERATED_BODY()

public:
	ABattleDirector();

	void Begin(const FString& StartBeat);
	virtual void Tick(float DeltaSeconds) override;

	void OnConfirm();
	bool RequestToken(AActor* Enemy);
	void ReleaseToken(AActor* Enemy);

	FString Phase;

private:
	enum class EWait : uint8 { None, Time, Confirm, WaveClear, BossDefeated };

	struct FStep
	{
		TFunction<void()> Run;
		EWait Wait = EWait::None;
		float Seconds = 0.f;
	};

	struct FTween
	{
		TFunction<void(float)> Apply;
		TFunction<void()> Done;
		float T = 0.f;
		float Duration = 1.f;
	};

	void Step(TFunction<void()> Run, EWait Wait = EWait::None, float Seconds = 0.f);
	void Wait(float Seconds) { Step([] {}, EWait::Time, Seconds); }
	void Tween(float Duration, TFunction<void(float)> Apply, TFunction<void()> Done = nullptr);
	void Advance();
	bool WaitSatisfied() const;

	void QueueIntro();
	void QueueWave1();
	void QueueWave2();
	void QueueBossIntro();
	void QueueBossFight();
	void QueueFinale();

	void SpawnAllies();
	void AlliesFall();
	AYakshaEnemy* SpawnSoldier(bool bCaptain);
	void StartWave(int32 Total);
	void UpdateObjective();
	void OnEnemyDied(AYakshaEnemy* Enemy);
	void OnHeadBroken(int32 Index, int32 Remaining);
	void OnPlayerDied();
	void LooseBrahmastra();
	void KillAllSoldiers();
	void WindGust();
	FString Stats() const;

	AHanumanCharacter* Hero() const;
	ASornHUD* Hud() const;
	void Cue(const FString& Id);

	TArray<FStep> Steps;
	int32 StepIndex = -1;
	float WaitLeft = 0.f;
	bool bConfirmed = false;
	bool bBossDefeated = false;
	bool bOver = false;
	bool bQuick = false;
	bool bAutoConfirm = false;

	TArray<FTween> Tweens;
	TArray<TWeakObjectPtr<AActor>> Tokens;
	int32 MaxTokens = 2;
	int32 Alive = 0;
	int32 WaveTotal = 0;
	int32 WaveKilled = 0;
	FString ObjectiveTh;
	FString ObjectiveEn;
	double StartTime = 0.0;

	UPROPERTY() TObjectPtr<AErawanBoss> Boss;
	UPROPERTY() TObjectPtr<AActor> Allies;
	UPROPERTY() TArray<TObjectPtr<UKhonFigureComponent>> AllyFigures;
};

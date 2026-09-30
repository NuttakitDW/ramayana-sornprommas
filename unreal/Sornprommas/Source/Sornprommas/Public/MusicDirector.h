#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MusicDirector.generated.h"

class UAudioComponent;

struct FMusicCue
{
	FString Id;
	FString Th;
	FString Rom;
	int32 Bpm = 100;
	FString Pattern;
	FString Use;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMusicBeat, int32 /*Index*/, bool /*bIsChap*/);

/**
 * Music cue system: each game state names the Khon piece that would accompany
 * it (mapping from lore/khon-performance/08-music-for-game-audio.md).
 *
 * If a SoundWave exists at /Game/Audio/Music/<cue_id> it is looped. Otherwise a
 * placeholder ฉิ่ง/ตะโพน/กลองทัด pulse is played at the cue's tempo. The
 * placeholder patterns are generic and are NOT the real หน้าทับ of each piece.
 * BeatPhase lets dancers sink (ยืดยุบ) on the beat.
 */
UCLASS()
class SORNPROMMAS_API AMusicDirector : public AActor
{
	GENERATED_BODY()

public:
	AMusicDirector();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void PlayCue(const FString& CueId);
	void Stop();
	void Sfx(const FString& Kind, float Pitch = 1.f, float Volume = 1.f);

	const FMusicCue* CurrentInfo() const;
	static const TArray<FMusicCue>& Cues();

	FString Current;
	float BeatPhase = 0.f;
	int32 BeatCount = 0;
	bool bUsingRecording = false;
	FOnMusicBeat OnBeat;

private:
	void OnTick(const FMusicCue& Info);
	void PlayPcm(const FString& Kind, float Pitch, float Volume);

	TMap<FString, TArray<uint8>> Pcm;

	UPROPERTY()
	TObjectPtr<UAudioComponent> RecordingPlayer;

	float Acc = 0.f;
	int32 TickIndex = 0;
};

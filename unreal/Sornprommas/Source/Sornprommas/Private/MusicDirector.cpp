#include "MusicDirector.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PercussionSynth.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
	constexpr int32 TicksPerBeat = 2;

	/** 8 ticks = 4 beats. C = ฉิ่ง (open), X = ฉับ (closed), T/t = ตะโพน high/low, K = กลองทัด. */
	const TMap<FString, TArray<FString>>& Patterns()
	{
		static const TMap<FString, TArray<FString>> P = {
			{TEXT("slow"), {TEXT("C K"), TEXT(""), TEXT("X"), TEXT(""), TEXT("C"), TEXT(""), TEXT("X t"), TEXT("")}},
			{TEXT("kraw"), {TEXT("C t"), TEXT("T"), TEXT("X T"), TEXT(""), TEXT("C t"), TEXT("T"), TEXT("X K"), TEXT("")}},
			{TEXT("choet"), {TEXT("C T"), TEXT("t"), TEXT("X T"), TEXT("t"), TEXT("C T"), TEXT("t"), TEXT("X K"), TEXT("t")}},
			{TEXT("choet_klong"), {TEXT("C K"), TEXT("T"), TEXT("X K"), TEXT("T"), TEXT("C K"), TEXT("T"), TEXT("X K"), TEXT("T t")}},
			{TEXT("ching_only"), {TEXT("C"), TEXT(""), TEXT("C"), TEXT(""), TEXT("C"), TEXT(""), TEXT("X"), TEXT("")}},
		};
		return P;
	}
}

const TArray<FMusicCue>& AMusicDirector::Cues()
{
	static const TArray<FMusicCue> C = {
		{TEXT("wa"), TEXT("เพลงวา"), TEXT("WA"), 66, TEXT("slow"), TEXT("หน้าชื่อบท / title")},
		{TEXT("kraw_nok"), TEXT("กราวนอก"), TEXT("KRAW NOK"), 96, TEXT("kraw"), TEXT("ยกทัพฝ่ายพระราม / Rama's army marches")},
		{TEXT("choet"), TEXT("เชิด"), TEXT("CHOET"), 150, TEXT("choet"), TEXT("รบ / battle")},
		{TEXT("choet_klong"), TEXT("เชิดกลอง"), TEXT("CHOET KLONG"), 168, TEXT("choet_klong"), TEXT("รบ / battle (intense)")},
		{TEXT("choet_ching"), TEXT("เชิดฉิ่ง"), TEXT("CHOET CHING"), 140, TEXT("ching_only"), TEXT("แผลงศร / loosing an arrow")},
		{TEXT("ling_lot"), TEXT("ลิงโลด"), TEXT("LING LOT"), 120, TEXT("kraw"), TEXT("หนุมานโกรธพระอินทร์แปลง")},
		{TEXT("ot_haep"), TEXT("โอดแหบ"), TEXT("OT HAEP"), 50, TEXT("slow"), TEXT("หนุมานสลบ / Hanuman falls")},
		{TEXT("khaek_bora_thet"), TEXT("แขกบรเทศ"), TEXT("KHAEK BORATHET"), 108, TEXT("kraw"), TEXT("ฟื้นคืน / revival")},
		{TEXT("kraw_ram_phama"), TEXT("กราวรำพม่า"), TEXT("KRAW RAM PHAMA"), 100, TEXT("kraw"), TEXT("ฝ่ายยักษ์ชนะ / demons triumph")},
	};
	return C;
}

AMusicDirector::AMusicDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMusicDirector::BeginPlay()
{
	Super::BeginPlay();
	for (const TCHAR* Kind : {TEXT("ching"), TEXT("chap"), TEXT("taphon_hi"), TEXT("taphon_lo"), TEXT("klong"),
		TEXT("whoosh"), TEXT("hit"), TEXT("clang")})
	{
		Pcm.Add(Kind, PercussionSynth::Make(Kind));
	}
	RecordingPlayer = NewObject<UAudioComponent>(this);
	RecordingPlayer->bAutoActivate = false;
	RecordingPlayer->bIsUISound = true;
	RecordingPlayer->RegisterComponent();
}

const FMusicCue* AMusicDirector::CurrentInfo() const
{
	return Cues().FindByPredicate([this](const FMusicCue& C) { return C.Id == Current; });
}

void AMusicDirector::PlayCue(const FString& CueId)
{
	const FMusicCue* Info = Cues().FindByPredicate([&](const FMusicCue& C) { return C.Id == CueId; });
	if (!Info || CueId == Current)
	{
		return;
	}
	Current = CueId;
	Acc = 0.f;
	TickIndex = 0;

	const FString Path = FString::Printf(TEXT("/Game/Audio/Music/%s.%s"), *CueId, *CueId);
	USoundWave* Recording = LoadObject<USoundWave>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	bUsingRecording = Recording != nullptr;
	RecordingPlayer->Stop();
	if (Recording)
	{
		Recording->bLooping = true;
		RecordingPlayer->SetSound(Recording);
		RecordingPlayer->Play();
	}
}

void AMusicDirector::Stop()
{
	Current.Reset();
	RecordingPlayer->Stop();
}

void AMusicDirector::Sfx(const FString& Kind, float Pitch, float Volume)
{
	PlayPcm(Kind, Pitch * FMath::FRandRange(0.94f, 1.06f), Volume);
}

void AMusicDirector::PlayPcm(const FString& Kind, float Pitch, float Volume)
{
	const TArray<uint8>* Data = Pcm.Find(Kind);
	if (!Data || Data->Num() == 0)
	{
		return;
	}
	// Procedural waves are consumed as they play, so each hit gets a fresh one.
	USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(this);
	Wave->SetSampleRate(PercussionSynth::SampleRate);
	Wave->NumChannels = 1;
	Wave->Duration = static_cast<float>(Data->Num() / 2) / PercussionSynth::SampleRate;
	Wave->SoundGroup = SOUNDGROUP_Default;
	Wave->bLooping = false;
	Wave->QueueAudio(Data->GetData(), Data->Num());
	UGameplayStatics::PlaySound2D(this, Wave, Volume, Pitch);
}

void AMusicDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FMusicCue* Info = CurrentInfo();
	if (!Info)
	{
		return;
	}
	// Music keeps real time through hit-stop and slow motion.
	const float RealDt = GetWorld()->DeltaRealTimeSeconds;
	const float TickLen = 60.f / Info->Bpm / TicksPerBeat;
	Acc += RealDt;
	while (Acc >= TickLen)
	{
		Acc -= TickLen;
		OnTick(*Info);
	}
	const float Within = ((TickIndex - 1 + TicksPerBeat) % TicksPerBeat) * TickLen + Acc;
	BeatPhase = FMath::Clamp(Within / (TickLen * TicksPerBeat), 0.f, 1.f);
}

void AMusicDirector::OnTick(const FMusicCue& Info)
{
	const TArray<FString>& Pattern = Patterns()[Info.Pattern];
	const FString& Step = Pattern[TickIndex % Pattern.Num()];
	if (TickIndex % TicksPerBeat == 0)
	{
		++BeatCount;
		OnBeat.Broadcast(BeatCount, Step.Contains(TEXT("X")));
	}
	++TickIndex;
	if (bUsingRecording)
	{
		return;
	}
	TArray<FString> Tokens;
	Step.ParseIntoArray(Tokens, TEXT(" "));
	for (const FString& T : Tokens)
	{
		if (T == TEXT("C")) PlayPcm(TEXT("ching"), 1.f, 0.5f);
		else if (T == TEXT("X")) PlayPcm(TEXT("chap"), 1.f, 0.5f);
		else if (T == TEXT("T")) PlayPcm(TEXT("taphon_hi"), 1.f, 0.7f);
		else if (T == TEXT("t")) PlayPcm(TEXT("taphon_lo"), 1.f, 0.7f);
		else if (T == TEXT("K")) PlayPcm(TEXT("klong"), 1.f, 0.8f);
	}
}

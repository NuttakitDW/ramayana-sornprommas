#include "YakshaEnemy.h"

#include "BattleDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HanumanCharacter.h"
#include "KhonFigureComponent.h"
#include "MusicDirector.h"
#include "SornCombat.h"
#include "SornGameMode.h"
#include "SornTypes.h"

namespace
{
	constexpr float StrikeRange = 230.f;
	constexpr float OrbitRange = 360.f;
}

AYakshaEnemy::AYakshaEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(45.f, 95.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bRunPhysicsWithNoController = true;
	Move->GravityScale = 2.65f;
	Move->MaxWalkSpeed = 2500.f;
	Move->GroundFriction = 0.f;
	Move->BrakingDecelerationWalking = 0.f;
	Move->BrakingDecelerationFalling = 0.f;
	Move->BrakingFrictionFactor = 0.f;
	Move->bOrientRotationToMovement = false;
	AutoPossessAI = EAutoPossessAI::Disabled;

	Figure = CreateDefaultSubobject<UKhonFigureComponent>(TEXT("Figure"));
	Figure->SetupAttachment(GetCapsuleComponent());
	Figure->SetRelativeLocation(FVector(0, 0, -95.f));
}

void AYakshaEnemy::Configure(const FLinearColor& Skin, bool bCaptain, const FString& NameTag)
{
	if (bCaptain)
	{
		MaxHp = 150.f;
		Speed = 320.f;
		Damage = 18.f;
		WindupTime = 0.8f;
		bPoise = true;
		BodyScale = 1.35f;
	}
	Hp = MaxHp;
	FKhonSpec Spec;
	Spec.Kind = EKhonKind::Yak;
	Spec.Skin = Skin;
	Spec.Cloth = bCaptain ? Sorn::Col::KhonRedDark : Sorn::Col::ClothDark;
	Spec.Trim = bCaptain ? Sorn::Col::GoldDeep : Sorn::Col::KhonRed;
	Spec.Crown = EKhonCrown::Yaksha;
	Spec.Weapon = EKhonWeapon::Club;
	Spec.Mouth = EKhonMouth::Grin;
	Figure->Build(Spec);
	Figure->SetRelativeScale3D(FVector(BodyScale));
	Figure->SetRelativeLocation(FVector(0, 0, -95.f));
	GetCapsuleComponent()->SetCapsuleSize(45.f * BodyScale, 95.f);

	if (!NameTag.IsEmpty())
	{
		UTextRenderComponent* Tag = NewObject<UTextRenderComponent>(this);
		Tag->SetupAttachment(GetCapsuleComponent());
		Tag->RegisterComponent();
		Tag->SetText(FText::FromString(NameTag));
		Tag->SetTextRenderColor(Sorn::Col::Gold.ToFColor(true));
		Tag->SetWorldSize(34.f);
		Tag->SetHorizontalAlignment(EHTA_Center);
		Tag->SetRelativeLocation(FVector(0, 0, 250.f));
		Tag->SetRelativeRotation(FRotator(0, 180.f, 0));
	}
}

void AYakshaEnemy::BeginPlay()
{
	Super::BeginPlay();
	OrbitDir = FMath::RandBool() ? 1.f : -1.f;
	T = FMath::FRandRange(0.6f, 1.2f);
	if (ASornGameMode* GM = ASornGameMode::Get(this))
	{
		GM->RegisterHittable(this);
	}
}

void AYakshaEnemy::EndPlay(const EEndPlayReason::Type Reason)
{
	ReleaseToken();
	if (ASornGameMode* GM = ASornGameMode::Get(this))
	{
		GM->UnregisterHittable(this);
	}
	Super::EndPlay(Reason);
}

TArray<FVector> AYakshaEnemy::HurtPoints() const
{
	return {GetActorLocation() + FVector(0, 0, 15.f * BodyScale)};
}

bool AYakshaEnemy::ReceiveHit(float Dmg, const FVector& From, float KnockAmt, int32 /*PointIndex*/)
{
	if (State == TEXT("dead"))
	{
		return false;
	}
	Hp -= Dmg;
	Figure->Flash(FLinearColor::White, 0.1f);
	const FVector Away = SornCombat::Flat(GetActorLocation() - From).GetSafeNormal();
	Knock = Away * KnockAmt * (bPoise ? 0.4f : 1.f);
	if (Hp <= 0.f)
	{
		Die();
	}
	else if (!(bPoise && State == TEXT("windup")))
	{
		ReleaseToken();
		State = TEXT("hurt");
		T = bPoise ? 0.12f : 0.28f;
		Figure->Play(TEXT("hurt"), T);
	}
	return true;
}

void AYakshaEnemy::Die()
{
	State = TEXT("dead");
	ReleaseToken();
	Figure->bDown = true;
	if (ASornGameMode* GM = ASornGameMode::Get(this))
	{
		GM->UnregisterHittable(this);
	}
	OnDied.Broadcast(this);
	Sfx(TEXT("klong"), BodyScale < 1.2f ? 1.3f : 0.8f);
	DeadT = 0.f;
}

void AYakshaEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Dt = DeltaSeconds;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (State == TEXT("dead"))
	{
		Knock = FMath::VInterpConstantTo(Knock, FVector::ZeroVector, Dt, 2000.f);
		Move->Velocity.X = Knock.X;
		Move->Velocity.Y = Knock.Y;
		DeadT += Dt;
		if (DeadT > 1.4f)
		{
			Figure->AddRelativeLocation(FVector(0, 0, -180.f * Dt));
		}
		if (DeadT > 2.6f)
		{
			Destroy();
		}
		return;
	}
	ASornGameMode* GM = ASornGameMode::Get(this);
	AHanumanCharacter* H = GM ? GM->Hanuman() : nullptr;
	if (!H)
	{
		return;
	}
	const FVector To = SornCombat::Flat(H->GetActorLocation() - GetActorLocation());
	const float Dist = To.Size();
	const FVector Dir = Dist > 1.f ? To / Dist : FVector::ForwardVector;
	FVector Wish = FVector::ZeroVector;
	T -= Dt;

	if (State == TEXT("enter"))
	{
		Wish = Dir * Speed * 1.4f;
		if (T <= 0.f) State = TEXT("chase");
	}
	else if (State == TEXT("chase"))
	{
		Wish = Chase(Dir, Dist);
	}
	else if (State == TEXT("windup"))
	{
		Face(Dir, Dt, 10.f);
		if (T <= 0.f)
		{
			State = TEXT("strike");
			T = 0.45f;
			bStrikeDone = false;
			Figure->Play(TEXT("strike"), 0.4f);
		}
	}
	else if (State == TEXT("strike"))
	{
		if (!bStrikeDone && T <= 0.33f)
		{
			bStrikeDone = true;
			TryHit(Dist, Dir);
		}
		if (T <= 0.f)
		{
			State = TEXT("recover");
			T = 0.7f;
		}
	}
	else if (State == TEXT("recover"))
	{
		if (T <= 0.f)
		{
			ReleaseToken();
			State = TEXT("chase");
		}
	}
	else if (State == TEXT("hurt") && T <= 0.f)
	{
		State = TEXT("chase");
	}

	Wish += Separation() * 250.f;
	Knock = FMath::VInterpConstantTo(Knock, FVector::ZeroVector, Dt, 2200.f);
	const FVector Cur = SornCombat::Flat(Move->Velocity) - Knock;
	const FVector Next = FMath::VInterpConstantTo(Cur, Wish, Dt, 3000.f) + Knock;
	Move->Velocity.X = Next.X;
	Move->Velocity.Y = Next.Y;
	if (State != TEXT("enter"))
	{
		SornCombat::ClampToArena(this, 80.f);
	}
	const bool bMoving = State == TEXT("enter") || State == TEXT("chase");
	if (bMoving)
	{
		Face(Dir, Dt, 8.f);
	}
	Figure->MoveRatio = bMoving ? FMath::Clamp(SornCombat::Flat(Move->Velocity).Size() / Speed, 0.f, 1.f) : 0.f;
	Figure->bGrounded = true;
}

FVector AYakshaEnemy::Chase(const FVector& Dir, float Dist)
{
	const AHanumanCharacter* H = ASornGameMode::Get(this)->Hanuman();
	if (!H || !H->IsAliveHanuman())
	{
		return FVector::ZeroVector;
	}
	if (Dist <= StrikeRange && T <= 0.f)
	{
		if (RequestToken())
		{
			State = TEXT("windup");
			T = WindupTime;
			Figure->Play(TEXT("windup"), WindupTime, true);
			Figure->Flash(FLinearColor(1.f, 0.45f, 0.1f), WindupTime * 0.6f);
			return FVector::ZeroVector;
		}
		T = 0.4f;
	}
	if (Dist > OrbitRange || bHasToken)
	{
		return Dir * Speed;
	}
	// Without a token, circle at a respectful distance (keeps fights readable).
	const FVector Tangent = FVector(-Dir.Y, Dir.X, 0.f) * OrbitDir;
	return Tangent * Speed * 0.55f + Dir * (Dist - OrbitRange) * 1.5f;
}

void AYakshaEnemy::TryHit(float Dist, const FVector& Dir)
{
	AHanumanCharacter* H = ASornGameMode::Get(this)->Hanuman();
	const float Cos = FVector::DotProduct(GetActorForwardVector(), Dir);
	if (H && Dist <= StrikeRange * BodyScale + 30.f && Cos > FMath::Cos(FMath::DegreesToRadians(70.f)))
	{
		H->TakeHit(Damage, GetActorLocation(), 700.f);
	}
	Sfx(TEXT("whoosh"), 0.7f, 0.7f);
}

void AYakshaEnemy::Face(const FVector& Dir, float Dt, float Rate)
{
	const float Target = SornCombat::YawToward(Dir);
	const float Yaw = FMath::FixedTurn(GetActorRotation().Yaw, Target, FMath::RadiansToDegrees(Rate) * Dt * 2.f);
	SetActorRotation(FRotator(0.f, Yaw, 0.f));
}

FVector AYakshaEnemy::Separation() const
{
	FVector Push = FVector::ZeroVector;
	for (TActorIterator<AYakshaEnemy> It(GetWorld()); It; ++It)
	{
		if (*It == this || !It->IsAlive())
		{
			continue;
		}
		const FVector D = SornCombat::Flat(GetActorLocation() - It->GetActorLocation());
		const float L = D.Size();
		if (L < 130.f && L > 0.1f)
		{
			Push += D / L * (130.f - L) / 100.f;
		}
	}
	if (const AHanumanCharacter* H = ASornGameMode::Get(this)->Hanuman())
	{
		const FVector D = SornCombat::Flat(GetActorLocation() - H->GetActorLocation());
		const float L = D.Size();
		if (L < 100.f && L > 0.1f)
		{
			Push += D / L * (100.f - L) / 100.f * 2.f;
		}
	}
	return Push;
}

bool AYakshaEnemy::RequestToken()
{
	ASornGameMode* GM = ASornGameMode::Get(this);
	bHasToken = !GM || !GM->Director || GM->Director->RequestToken(this);
	return bHasToken;
}

void AYakshaEnemy::ReleaseToken()
{
	if (!bHasToken)
	{
		return;
	}
	bHasToken = false;
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Director)
	{
		GM->Director->ReleaseToken(this);
	}
}

void AYakshaEnemy::Sfx(const FString& Kind, float Pitch, float Volume)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
	{
		GM->Music->Sfx(Kind, Pitch, Volume);
	}
}

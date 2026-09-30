#include "HanumanCharacter.h"

#include "BattleDirector.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "KhonFigureComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MusicDirector.h"
#include "SornCombat.h"
#include "SornFx.h"
#include "SornGameMode.h"
#include "SornHUD.h"
#include "SornHittable.h"
#include "SornTypes.h"

namespace
{
	constexpr float Speed = 720.f;
	constexpr float Accel = 4200.f;
	constexpr float JumpV = 1050.f;
	constexpr float DashSpeed = 1700.f;
	constexpr float DashTime = 0.2f;
	constexpr float DashCooldown = 0.45f;
	constexpr float GiantTime = 9.f;
	constexpr float GiantScale = 1.7f;
	constexpr float PowerPerHit = 7.f;

	const FHanumanAttack& AttackFor(FName Id)
	{
		static const FHanumanAttack Light1{TEXT("swing_a"), 0.36f, 0.13f, 12.f, 260.f, 130.f, 300.f, 300.f, 0.12f};
		static const FHanumanAttack Light2{TEXT("swing_b"), 0.36f, 0.13f, 13.f, 260.f, 130.f, 300.f, 300.f, 0.12f};
		static const FHanumanAttack Light3{TEXT("slam"), 0.52f, 0.27f, 24.f, 290.f, 160.f, 900.f, 400.f, 0.35f};
		static const FHanumanAttack Heavy{TEXT("thrust"), 0.58f, 0.27f, 32.f, 340.f, 60.f, 1100.f, 1000.f, 0.3f};
		static const FHanumanAttack Recovery{NAME_None, 0.22f, 99.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
		if (Id == TEXT("light1")) return Light1;
		if (Id == TEXT("light2")) return Light2;
		if (Id == TEXT("light3")) return Light3;
		if (Id == TEXT("heavy")) return Heavy;
		return Recovery;
	}
}

AHanumanCharacter::AHanumanCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->GravityScale = 2.65f;
	Move->MaxWalkSpeed = 2500.f;
	Move->GroundFriction = 0.f;
	Move->BrakingDecelerationWalking = 0.f;
	Move->BrakingDecelerationFalling = 0.f;
	Move->BrakingFrictionFactor = 0.f;
	Move->AirControl = 0.f;
	Move->bOrientRotationToMovement = false;
	Move->JumpZVelocity = JumpV;

	Figure = CreateDefaultSubobject<UKhonFigureComponent>(TEXT("Figure"));
	Figure->SetupAttachment(GetCapsuleComponent());
	Figure->SetRelativeLocation(FVector(0, 0, -90.f));

	Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	Arm->SetupAttachment(GetCapsuleComponent());
	Arm->SetUsingAbsoluteLocation(true);
	Arm->bUsePawnControlRotation = true;
	Arm->bDoCollisionTest = false;
	Arm->TargetArmLength = ArmLen;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Arm);
	Camera->SetFieldOfView(62.f);

	Aura = CreateDefaultSubobject<UPointLightComponent>(TEXT("Aura"));
	Aura->SetupAttachment(GetCapsuleComponent());
	Aura->SetRelativeLocation(FVector(0, 0, 70.f));
	Aura->SetIntensity(0.f);
	Aura->SetLightColor(FLinearColor(0.6f, 0.8f, 1.f));
	Aura->SetAttenuationRadius(600.f);
}

void AHanumanCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (const ASornGameMode* GM = ASornGameMode::Get(this))
	{
		bAutobot = GM->bAutobot;
	}
	FKhonSpec Spec;
	Spec.Kind = EKhonKind::Ling;
	Spec.Skin = Sorn::Col::HanumanWhite;
	Spec.Cloth = Sorn::Col::KhonRed;
	Spec.Trim = Sorn::Col::KhonRedDark;
	Spec.Crown = EKhonCrown::Hanuman;
	Spec.Weapon = EKhonWeapon::Trident;
	Spec.Mouth = EKhonMouth::Open;
	Spec.bTail = true;
	Figure->Build(Spec);
	FacingYaw = GetActorRotation().Yaw;
	LookPoint = GetActorLocation();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator(-17.f, FacingYaw, 0.f));
		PC->PlayerCameraManager->ViewPitchMin = -50.f;
		PC->PlayerCameraManager->ViewPitchMax = 3.f;
	}
}

// ------------------------------------------------------------------ input

void AHanumanCharacter::BuildInput()
{
	if (Mapping)
	{
		return;
	}
	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Hanuman"));
	auto MakeAction = [this](FName Name, EInputActionValueType Type)
	{
		UInputAction* A = NewObject<UInputAction>(this, Name);
		A->ValueType = Type;
		Actions.Add(Name, A);
		return A;
	};
	auto Map = [this](UInputAction* A, const FKey& Key, bool bSwizzle = false, bool bNegate = false, bool bNegateYOnly = false)
	{
		FEnhancedActionKeyMapping& M = Mapping->MapKey(A, Key);
		if (bSwizzle)
		{
			UInputModifierSwizzleAxis* S = NewObject<UInputModifierSwizzleAxis>(this);
			S->Order = EInputAxisSwizzle::YXZ;
			M.Modifiers.Add(S);
		}
		if (bNegate || bNegateYOnly)
		{
			UInputModifierNegate* N = NewObject<UInputModifierNegate>(this);
			N->bX = !bNegateYOnly;
			N->bZ = !bNegateYOnly;
			M.Modifiers.Add(N);
		}
	};

	UInputAction* Move = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	Map(Move, EKeys::W, true);
	Map(Move, EKeys::S, true, true);
	Map(Move, EKeys::A, false, true);
	Map(Move, EKeys::D);
	Map(Move, EKeys::Gamepad_Left2D);

	UInputAction* Look = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	Map(Look, EKeys::Mouse2D, false, false, true);
	Map(Look, EKeys::Gamepad_Right2D, false, false, true);
	Map(Look, EKeys::Right);
	Map(Look, EKeys::Left, false, true);

	auto Button = [&](FName Name, std::initializer_list<FKey> Keys)
	{
		UInputAction* A = MakeAction(Name, EInputActionValueType::Boolean);
		for (const FKey& K : Keys)
		{
			Map(A, K);
		}
	};
	Button(TEXT("IA_Attack"), {EKeys::J, EKeys::LeftMouseButton, EKeys::Gamepad_FaceButton_Left});
	Button(TEXT("IA_Heavy"), {EKeys::K, EKeys::RightMouseButton, EKeys::Gamepad_FaceButton_Top});
	Button(TEXT("IA_Jump"), {EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom});
	Button(TEXT("IA_Dash"), {EKeys::LeftShift, EKeys::L, EKeys::Gamepad_FaceButton_Right});
	Button(TEXT("IA_Special"), {EKeys::Q, EKeys::Gamepad_RightShoulder});
	Button(TEXT("IA_Confirm"), {EKeys::Enter, EKeys::SpaceBar, EKeys::J, EKeys::LeftMouseButton, EKeys::Gamepad_FaceButton_Bottom});
	Button(TEXT("IA_Restart"), {EKeys::R, EKeys::Gamepad_Special_Right});
	Button(TEXT("IA_Help"), {EKeys::H});
}

void AHanumanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BuildInput();
	UEnhancedInputComponent* In = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!In)
	{
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input component missing; check DefaultInput.ini"));
		return;
	}
	In->BindAction(Actions[TEXT("IA_Move")], ETriggerEvent::Triggered, this, &AHanumanCharacter::OnMove);
	In->BindAction(Actions[TEXT("IA_Move")], ETriggerEvent::Completed, this, &AHanumanCharacter::OnMove);
	In->BindAction(Actions[TEXT("IA_Look")], ETriggerEvent::Triggered, this, &AHanumanCharacter::OnLook);
	In->BindAction(Actions[TEXT("IA_Attack")], ETriggerEvent::Started, this, &AHanumanCharacter::OnAttack);
	In->BindAction(Actions[TEXT("IA_Heavy")], ETriggerEvent::Started, this, &AHanumanCharacter::OnHeavy);
	In->BindAction(Actions[TEXT("IA_Jump")], ETriggerEvent::Started, this, &AHanumanCharacter::OnJumpPressed);
	In->BindAction(Actions[TEXT("IA_Dash")], ETriggerEvent::Started, this, &AHanumanCharacter::OnDash);
	In->BindAction(Actions[TEXT("IA_Special")], ETriggerEvent::Started, this, &AHanumanCharacter::OnSpecial);
	In->BindAction(Actions[TEXT("IA_Confirm")], ETriggerEvent::Started, this, &AHanumanCharacter::OnConfirm);
	In->BindAction(Actions[TEXT("IA_Restart")], ETriggerEvent::Started, this, &AHanumanCharacter::OnRestart);
	In->BindAction(Actions[TEXT("IA_Help")], ETriggerEvent::Started, this, &AHanumanCharacter::OnToggleHelp);
}

void AHanumanCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	BuildInput();
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Sub->ClearAllMappings();
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

void AHanumanCharacter::OnMove(const FInputActionValue& V) { MoveInput = V.Get<FVector2D>(); }

void AHanumanCharacter::OnLook(const FInputActionValue& V)
{
	const FVector2D L = V.Get<FVector2D>();
	AddControllerYawInput(L.X * 0.35f);
	AddControllerPitchInput(L.Y * 0.25f);
}

void AHanumanCharacter::Press(FName Action)
{
	if (bInputEnabled && !bAutobot)
	{
		Pressed.Add(Action);
	}
}

void AHanumanCharacter::OnAttack() { Press(TEXT("attack")); }
void AHanumanCharacter::OnHeavy() { Press(TEXT("heavy")); }
void AHanumanCharacter::OnJumpPressed() { Press(TEXT("jump")); }
void AHanumanCharacter::OnDash() { Press(TEXT("dash")); }
void AHanumanCharacter::OnSpecial() { Press(TEXT("special")); }

void AHanumanCharacter::OnConfirm()
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Director)
	{
		GM->Director->OnConfirm();
	}
}

void AHanumanCharacter::OnRestart()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AHanumanCharacter::OnToggleHelp()
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ASornHUD* Hud = Cast<ASornHUD>(PC->GetHUD()))
		{
			Hud->ToggleHelp();
		}
	}
}

bool AHanumanCharacter::Consume(FName Action)
{
	if (!bInputEnabled)
	{
		return false;
	}
	return Pressed.Remove(Action) > 0;
}

// ------------------------------------------------------------------ tick

void AHanumanCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Dt = DeltaSeconds;
	if (bAutobot && bInputEnabled)
	{
		TickBot();
	}
	TickTimers(Dt);

	FVector Wish = FVector::ZeroVector;
	if (bInputEnabled)
	{
		if (bAutobot)
		{
			Wish = FVector(MoveInput.X, MoveInput.Y, 0.f); // bot writes world XY into MoveInput
		}
		else if (Controller)
		{
			const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
			const FVector Fwd = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
			const FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
			Wish = Fwd * MoveInput.Y + Right * MoveInput.X;
		}
		Wish = Wish.GetClampedToMaxSize(1.f);
	}

	if (State == TEXT("normal")) TickNormal(Dt, Wish);
	else if (State == TEXT("attack")) TickAttack(Dt);
	else if (State == TEXT("plunge")) TickPlunge();
	else if (State == TEXT("dash")) TickDash(Dt);
	else if (State == TEXT("special")) TickSpecial(Dt);
	else if (State != TEXT("puppet")) Decelerate(Dt, 1000.f);

	if (State != TEXT("puppet"))
	{
		SornCombat::ClampToArena(this, 60.f);
		const FRotator Cur = GetActorRotation();
		SetActorRotation(FRotator(0.f, FMath::FixedTurn(Cur.Yaw, FacingYaw, 1080.f * Dt), 0.f));
	}
	Pressed.Reset();

	const float FlatSpeed = SornCombat::Flat(GetVelocity()).Size();
	Figure->MoveRatio = State == TEXT("normal") ? FMath::Clamp(FlatSpeed / Speed, 0.f, 1.f) : 0.f;
	Figure->bGrounded = GetCharacterMovement()->IsMovingOnGround() || State == TEXT("down") || State == TEXT("puppet");

	FigureScale = FMath::FInterpTo(FigureScale, TargetFigureScale, Dt, 8.f);
	Figure->SetRelativeScale3D(FVector(FigureScale));
	TickCamera(DeltaSeconds);
}

void AHanumanCharacter::TickTimers(float Dt)
{
	DashCd = FMath::Max(0.f, DashCd - Dt);
	InvulnT = FMath::Max(0.f, InvulnT - Dt);
	ComboTimer -= Dt;
	if (ComboTimer <= 0.f)
	{
		ComboCount = 0;
	}
	if (State == TEXT("hurt"))
	{
		HurtT -= Dt;
		if (HurtT <= 0.f)
		{
			State = TEXT("normal");
		}
	}
	if (GiantLeft > 0.f)
	{
		GiantLeft -= Dt;
		Aura->SetIntensity(9000.f + FMath::Sin(GetWorld()->GetTimeSeconds() * 10.f) * 2000.f);
		if (GiantLeft <= 0.f)
		{
			SetGiant(false);
		}
	}
}

void AHanumanCharacter::SetHorizontalVelocity(const FVector& V)
{
	UCharacterMovementComponent* M = GetCharacterMovement();
	M->Velocity.X = V.X;
	M->Velocity.Y = V.Y;
}

void AHanumanCharacter::Decelerate(float Dt, float Rate)
{
	const FVector H = SornCombat::Flat(GetCharacterMovement()->Velocity);
	SetHorizontalVelocity(FMath::VInterpConstantTo(H, FVector::ZeroVector, Dt, Rate * 4.f));
}

void AHanumanCharacter::TickNormal(float Dt, const FVector& Wish)
{
	const float S = Speed * (GiantLeft > 0.f ? 1.15f : 1.f);
	const FVector H = SornCombat::Flat(GetCharacterMovement()->Velocity);
	SetHorizontalVelocity(FMath::VInterpConstantTo(H, Wish * S, Dt, Accel));
	if (Wish.Size() > 0.1f)
	{
		FacingYaw = SornCombat::YawToward(Wish);
	}
	const bool bGround = GetCharacterMovement()->IsMovingOnGround();
	if (Consume(TEXT("jump")) && bGround)
	{
		LaunchCharacter(FVector(0, 0, JumpV), false, true);
		Sfx(TEXT("whoosh"), 0.8f);
	}
	if (Consume(TEXT("attack")))
	{
		if (bGround) StartAttack(TEXT("light1"));
		else StartPlunge();
	}
	else if (Consume(TEXT("heavy")) && bGround)
	{
		StartAttack(TEXT("heavy"));
	}
	else if (Consume(TEXT("dash")) && DashCd <= 0.f)
	{
		StartDash(Wish);
	}
	else if (Consume(TEXT("special")) && Power >= 100.f && GiantLeft <= 0.f)
	{
		StartSpecial();
	}
}

// ------------------------------------------------------------------ attacks

float AHanumanCharacter::ScaleMul() const
{
	return GiantLeft > 0.f ? GiantScale : 1.f;
}

void AHanumanCharacter::StartAttack(FName Id)
{
	AttackId = Id;
	Attack = AttackFor(Id);
	AttackT = 0.f;
	bAttackHitDone = false;
	Queued = NAME_None;
	State = TEXT("attack");
	AutoFace();
	SetActorRotation(FRotator(0.f, FacingYaw, 0.f));
	SetHorizontalVelocity(Forward() * Attack.Lunge);
	Figure->Play(Attack.Pose, Attack.Len);
	Sfx(TEXT("whoosh"), Id.ToString().StartsWith(TEXT("light")) ? 1.2f : 0.9f, 0.6f);
}

void AHanumanCharacter::AutoFace()
{
	const FSornStrike Hit = SornCombat::Nearest(this, GetActorLocation(), 550.f, 300.f);
	if (Hit.Index >= 0)
	{
		FacingYaw = SornCombat::YawToward(SornCombat::Flat(Hit.Point - GetActorLocation()));
	}
}

void AHanumanCharacter::TickAttack(float Dt)
{
	AttackT += Dt;
	Decelerate(Dt, 900.f);
	if (Consume(TEXT("attack"))) Queued = TEXT("light");
	else if (Consume(TEXT("heavy"))) Queued = TEXT("heavy");
	if (!bAttackHitDone && AttackT >= Attack.HitAt)
	{
		bAttackHitDone = true;
		ResolveHit(Attack);
	}
	if (bAttackHitDone && Consume(TEXT("dash")))
	{
		StartDash(FVector::ZeroVector);
		return;
	}
	if (AttackT >= Attack.Len)
	{
		FinishAttack();
	}
}

void AHanumanCharacter::FinishAttack()
{
	if (Queued == TEXT("light") && AttackId == TEXT("plunge"))
	{
		StartAttack(TEXT("light1"));
	}
	else if (Queued == TEXT("light") && AttackId != TEXT("light3") && AttackId != TEXT("heavy"))
	{
		StartAttack(AttackId == TEXT("light1") ? TEXT("light2") : TEXT("light3"));
	}
	else if (Queued == TEXT("heavy"))
	{
		StartAttack(TEXT("heavy"));
	}
	else
	{
		State = TEXT("normal");
	}
}

void AHanumanCharacter::ResolveHit(const FHanumanAttack& Atk)
{
	const float S = ScaleMul();
	const FVector Origin = GetActorLocation() + FVector(0, 0, 10.f * S);
	for (const FSornStrike& Hit : SornCombat::Sweep(this, Origin, Forward(), Atk.Reach * S, Atk.Arc, 230.f * S))
	{
		ApplyStrike(Hit, Atk.Dmg * (GiantLeft > 0.f ? 1.5f : 1.f), Atk.Knock, Atk.Shake);
	}
}

void AHanumanCharacter::ApplyStrike(const FSornStrike& S, float Dmg, float Knock, float ShakeAmt)
{
	ISornHittable* H = Cast<ISornHittable>(S.Target.Get());
	if (!H)
	{
		return;
	}
	const bool bLanded = H->ReceiveHit(Dmg, GetActorLocation(), Knock, S.Index);
	ASornGameMode* GM = ASornGameMode::Get(this);
	ASornFx* Fx = GM ? GM->Fx.Get() : nullptr;
	if (bLanded)
	{
		++ComboCount;
		MaxCombo = FMath::Max(MaxCombo, ComboCount);
		ComboTimer = 2.2f;
		Power = FMath::Min(100.f, Power + PowerPerHit);
		if (Fx)
		{
			Fx->Sparks(S.Point, Sorn::Col::Gold, 16);
			Fx->Number(S.Point + FVector(0, 0, 40), Dmg, Sorn::Col::UiPale);
			Fx->HitStop(0.05f + ShakeAmt * 0.12f);
		}
		Sfx(TEXT("hit"), 1.f + FMath::FRand() * 0.2f);
		Shake(ShakeAmt);
	}
	else if (Fx)
	{
		Fx->Sparks(S.Point, Sorn::Col::UiPale, 8, 400.f);
		Sfx(TEXT("clang"), 1.4f, 0.5f);
	}
}

void AHanumanCharacter::StartPlunge()
{
	State = TEXT("plunge");
	const FVector V = GetCharacterMovement()->Velocity;
	GetCharacterMovement()->Velocity = FVector(V.X * 0.3f, V.Y * 0.3f, -2400.f);
	Figure->Play(TEXT("plunge"), 0.4f, true);
	Sfx(TEXT("whoosh"), 0.7f);
}

void AHanumanCharacter::TickPlunge()
{
	GetCharacterMovement()->Velocity.Z = -2400.f;
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}
	const float S = ScaleMul();
	for (const FSornStrike& Hit : SornCombat::Sweep(this, GetActorLocation(), Forward(), 280.f * S, 360.f, 260.f * S))
	{
		ApplyStrike(Hit, 22.f * (GiantLeft > 0.f ? 1.5f : 1.f), 800.f, 0.3f);
	}
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Fx)
	{
		GM->Fx->Ring(GetActorLocation() - FVector(0, 0, 88.f), 320.f * S, Sorn::Col::Gold);
	}
	Sfx(TEXT("klong"), 1.2f);
	Shake(0.3f);
	Figure->ClearAction();
	AttackId = TEXT("plunge");
	Attack = AttackFor(NAME_None);
	AttackT = 0.f;
	bAttackHitDone = true;
	Queued = NAME_None;
	State = TEXT("attack");
}

void AHanumanCharacter::StartDash(const FVector& Wish)
{
	FVector Dir = Wish.Size() > 0.1f ? Wish : Forward();
	Dir = SornCombat::Flat(Dir).GetSafeNormal();
	DashDir = Dir;
	FacingYaw = SornCombat::YawToward(Dir);
	DashT = DashTime;
	DashCd = DashCooldown;
	InvulnT = DashTime + 0.08f;
	State = TEXT("dash");
	Figure->ClearAction();
	Sfx(TEXT("whoosh"), 1.5f, 0.8f);
}

void AHanumanCharacter::TickDash(float Dt)
{
	SetHorizontalVelocity(DashDir * DashSpeed);
	DashT -= Dt;
	if (DashT <= 0.f)
	{
		SetHorizontalVelocity(DashDir * DashSpeed * 0.3f);
		State = TEXT("normal");
	}
}

// ------------------------------------------------------------------ นิมิตกาย

void AHanumanCharacter::StartSpecial()
{
	State = TEXT("special");
	SpecialT = 0.f;
	Power = 0.f;
	SetHorizontalVelocity(FVector::ZeroVector);
	Figure->Play(TEXT("special"), 0.7f);
	Sfx(TEXT("klong"), 0.7f);
}

void AHanumanCharacter::TickSpecial(float Dt)
{
	const float Before = SpecialT;
	SpecialT += Dt;
	if (Before < 0.4f && SpecialT >= 0.4f)
	{
		SetGiant(true);
		for (const FSornStrike& Hit : SornCombat::Sweep(this, GetActorLocation(), Forward(), 700.f, 360.f, 600.f))
		{
			ApplyStrike(Hit, 38.f, 1300.f, 0.6f);
		}
		if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Fx)
		{
			GM->Fx->Ring(GetActorLocation() - FVector(0, 0, 88.f), 800.f, Sorn::Col::UiSignal, 0.6f);
		}
	}
	if (SpecialT >= 0.75f)
	{
		State = TEXT("normal");
	}
}

void AHanumanCharacter::SetGiant(bool bOn)
{
	GiantLeft = bOn ? GiantTime : 0.f;
	TargetFigureScale = bOn ? GiantScale : 1.f;
	if (!bOn)
	{
		Aura->SetIntensity(0.f);
	}
}

// ------------------------------------------------------------------ damage

bool AHanumanCharacter::TakeHit(float Dmg, const FVector& From, float Knock)
{
	static const TSet<FName> Immune = {TEXT("dead"), TEXT("down"), TEXT("cutscene"), TEXT("puppet"), TEXT("special"), TEXT("dash")};
	if (Immune.Contains(State) || InvulnT > 0.f)
	{
		return false;
	}
	const bool bGiant = GiantLeft > 0.f;
	Hp -= Dmg * (bGiant ? 0.6f : 1.f);
	++HitsTaken;
	ComboCount = 0;
	Figure->Flash(Sorn::Col::Danger, 0.14f);
	Shake(0.4f);
	Sfx(TEXT("hit"), 0.7f);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ASornHUD* Hud = Cast<ASornHUD>(PC->GetHUD()))
		{
			Hud->HurtFlash();
		}
	}
	const FVector Away = SornCombat::Flat(GetActorLocation() - From).GetSafeNormal();
	if (Hp <= 0.f)
	{
		Hp = 0.f;
		State = TEXT("dead");
		Figure->bDown = true;
		bInputEnabled = false;
		OnDied.Broadcast();
		return true;
	}
	if (!bGiant)
	{
		State = TEXT("hurt");
		HurtT = 0.32f;
		InvulnT = 0.45f;
		Figure->Play(TEXT("hurt"), 0.3f);
		LaunchCharacter(Away * Knock + FVector(0, 0, 200.f), true, true);
	}
	return true;
}

// ------------------------------------------------------------------ cutscene API

void AHanumanCharacter::EnterCutscene()
{
	bInputEnabled = false;
	State = TEXT("cutscene");
	Figure->ClearAction();
	MoveInput = FVector2D::ZeroVector;
}

void AHanumanCharacter::LeaveCutscene()
{
	bInputEnabled = true;
	State = TEXT("normal");
}

void AHanumanCharacter::FacePoint(const FVector& P)
{
	FacingYaw = SornCombat::YawToward(SornCombat::Flat(P - GetActorLocation()));
}

void AHanumanCharacter::KnockDown()
{
	State = TEXT("down");
	Figure->bDown = true;
	Figure->Play(TEXT("hurt"), 0.3f);
	SetGiant(false);
}

void AHanumanCharacter::Revive()
{
	Figure->bDown = false;
	State = TEXT("cutscene");
	Hp = FMath::Max(Hp, MaxHp * 0.5f);
}

void AHanumanCharacter::SetPuppet(bool bOn)
{
	UCharacterMovementComponent* M = GetCharacterMovement();
	if (bOn)
	{
		State = TEXT("puppet");
		M->StopMovementImmediately();
		M->SetMovementMode(MOVE_Flying);
		M->Velocity = FVector::ZeroVector;
		Figure->bGrounded = false;
	}
	else
	{
		M->SetMovementMode(MOVE_Falling);
		State = TEXT("cutscene");
	}
}

// ------------------------------------------------------------------ camera

void AHanumanCharacter::TickCamera(float DeltaSeconds)
{
	const float RealDt = GetWorld()->DeltaRealTimeSeconds;
	FVector Look = GetActorLocation() + FVector(0, 0, 50.f);
	float WantLen = 700.f;
	if (AActor* F = CameraFocus.Get())
	{
		Look = FMath::Lerp(Look, F->GetActorLocation(), 0.42f);
		WantLen = 1400.f;
	}
	ArmLen = FMath::Lerp(ArmLen, WantLen, 1.f - FMath::Exp(-3.f * RealDt));
	LookPoint = LookPoint.IsZero() ? Look : FMath::Lerp(LookPoint, Look, 1.f - FMath::Exp(-10.f * RealDt));
	Arm->SetWorldLocation(LookPoint);
	Arm->TargetArmLength = ArmLen;

	Trauma = FMath::Max(0.f, Trauma - RealDt * 1.8f);
	const float S = Trauma * Trauma;
	Camera->SetRelativeLocation(FVector(0.f, FMath::FRandRange(-1.f, 1.f) * 35.f * S, FMath::FRandRange(-1.f, 1.f) * 35.f * S));
}

// ------------------------------------------------------------------ autobot

void AHanumanCharacter::TickBot()
{
	MoveInput = FVector2D::ZeroVector;
	if (Power >= 100.f && GiantLeft <= 0.f)
	{
		Pressed.Add(TEXT("special"));
		return;
	}
	const FSornStrike Target = SornCombat::Nearest(this, GetActorLocation(), 8000.f, 320.f);
	const FVector Goal = Target.Index >= 0 ? Target.Point : FVector::ZeroVector;
	const FVector To = SornCombat::Flat(Goal - GetActorLocation());
	const float Dist = To.Size();
	if (Target.Index < 0 && Dist < 300.f)
	{
		return;
	}
	if (Dist > 230.f || Target.Index < 0)
	{
		const FVector D = To.GetSafeNormal();
		MoveInput = FVector2D(D.X, D.Y);
		if (Dist > 800.f && FMath::FRand() < 0.01f)
		{
			Pressed.Add(TEXT("dash"));
		}
	}
	else
	{
		Pressed.Add(FMath::FRand() < 0.12f ? TEXT("heavy") : TEXT("attack"));
	}
	if (FMath::FRand() < 0.004f)
	{
		Pressed.Add(TEXT("jump"));
	}
}

void AHanumanCharacter::Sfx(const FString& Kind, float Pitch, float Volume)
{
	if (ASornGameMode* GM = ASornGameMode::Get(this); GM && GM->Music)
	{
		GM->Music->Sfx(Kind, Pitch, Volume);
	}
}

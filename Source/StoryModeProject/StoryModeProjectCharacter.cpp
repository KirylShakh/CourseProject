// Copyright Epic Games, Inc. All Rights Reserved.

#include "StoryModeProjectCharacter.h"
#include "FlyerCharacterMovementComponent.h"
#include "FlyerAttributeSet.h"
#include "Weapons/WeaponActor.h"
#include "StoryModeProjectPlayerState.h"
#include "StoryModeProjectGameMode.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"

//////////////////////////////////////////////////////////////////////////
// AStoryModeProjectCharacter

AStoryModeProjectCharacter::AStoryModeProjectCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UFlyerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	MuzzlePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(GetMesh());
	MuzzlePoint->SetRelativeLocation(FVector(0.f, 30.f, 130.f));
	MuzzlePoint->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AStoryModeProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetPlayerState();
	if (HasAuthority() && IsValid(GetAbilitySystemComponent()))
	{
		for (int i = 0; i < GameplayAbilities.Num(); i++)
		{
			GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GameplayAbilities[i], 1, i, this));
		}

		AttributeSet = GetAbilitySystemComponent()->AddSet<UFlyerAttributeSet>();
		Health = AttributeSet->GetMaxHealth();
	}
}

float AStoryModeProjectCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		if (bDead)
		{
			return 0;
		}

		Health = FMath::Clamp(Health - DamageAmount, AttributeSet->GetMinHealth(), AttributeSet->GetMaxHealth());
		OnHealthUpdate();

		if (bDead)
		{
			AStoryModeProjectCharacter* KilledBy = Cast<AStoryModeProjectCharacter>(DamageCauser);
			if (KilledBy)
			{
				AStoryModeProjectPlayerState* KillerPlayerState = Cast<AStoryModeProjectPlayerState>(KilledBy->GetPlayerState());
				if (KillerPlayerState)
				{
					KillerPlayerState->KillCount += 1;
					AStoryModeProjectGameMode* GameMode = Cast<AStoryModeProjectGameMode>(GetWorld()->GetAuthGameMode());
					if (GameMode)
					{
						GameMode->PlayerKilled(KilledBy);
					}
				}
			}
			else
			{
				AStoryModeProjectPlayerState* SelfPlayerState = Cast<AStoryModeProjectPlayerState>(GetPlayerState());
				if (SelfPlayerState)
				{
					SelfPlayerState->KillCount -= 1;
				}
			}

			AStoryModeProjectGameMode* GameMode = Cast<AStoryModeProjectGameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode)
			{
				GameMode->OnPlayerDied(KilledBy, this);
			}
		}

		return DamageAmount;
	}

	return 0;
}

void AStoryModeProjectCharacter::ShieldDestroyed()
{
	OnStopShielding();
}

void AStoryModeProjectCharacter::OnRep_Health()
{
	OnHealthUpdate();
}

void AStoryModeProjectCharacter::OnHealthUpdate()
{
	if (GetHealth() <= 0 && !bDead)
	{
		bDead = true;

		// If was in flying mode - change it to falling mode
		UFlyerCharacterMovementComponent* MoveCmp = Cast<UFlyerCharacterMovementComponent>(GetMovementComponent());
		if (MoveCmp && MoveCmp->IsFlying())
		{
			MoveCmp->Fly();
		}

		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ragdoll"));

		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->Stop();

		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);

		GetMesh()->SetAllBodiesPhysicsBlendWeight(1.f);
		GetMesh()->SetPhysicsBlendWeight(1.f);

		GetMesh()->WakeAllRigidBodies();

		OnDied();
	}

	if (GetLocalRole() >= ENetRole::ROLE_AutonomousProxy)
	{
		OnHealthUpdated();
	}
}

void AStoryModeProjectCharacter::OnRep_Aiming()
{
	OnAimingUpdate();
}

void AStoryModeProjectCharacter::OnAimingUpdate()
{
	if (bAiming)
	{
		bUseControllerRotationYaw = true;
	}
	else
	{
		bUseControllerRotationYaw = false;

		Server_CancelAbility(ChargedTags);
	}
}

void AStoryModeProjectCharacter::Server_Aim_Implementation(bool bWantsToAim)
{
	bAiming = bWantsToAim;
	OnAimingUpdate();
}

bool AStoryModeProjectCharacter::CanMove()
{
	return !bAiming;
}

void AStoryModeProjectCharacter::Server_ActivateAbility_Implementation(int32 InputCode)
{
	ActivateAbility(InputCode);

	// Store camera location for its simulated proxies on other clients - Maybe look for a better place to do it
	RepCameraLocation = FollowCamera->GetComponentLocation();
	RepCameraForwardVector = FollowCamera->GetForwardVector();
}

void AStoryModeProjectCharacter::Server_DeactivateAbility_Implementation(int32 InputCode)
{
	DeactivateAbility(InputCode);
}

void AStoryModeProjectCharacter::Server_CancelAbility_Implementation(const FGameplayTagContainer CancelWithTags)
{
	CancelAbility(CancelWithTags);
}

FHitResult AStoryModeProjectCharacter::GetAimHitResult()
{
	const FVector CameraLocation = GetLocalRole() == ENetRole::ROLE_SimulatedProxy ? RepCameraLocation : FollowCamera->GetComponentLocation();
	const FVector CameraForwardVector = GetLocalRole() == ENetRole::ROLE_SimulatedProxy ? RepCameraForwardVector : FollowCamera->GetForwardVector();

	const FVector Start = CameraLocation;
	const FVector End = Start + CameraForwardVector * AimRange;

	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	return Hit;
}

const FVector AStoryModeProjectCharacter::GetMuzzlePoint()
{
	return MuzzlePoint ? MuzzlePoint->GetComponentLocation() : FVector::ZeroVector;
}

void AStoryModeProjectCharacter::OnRep_Firing()
{
	if (FiringInfo.bFiring && FiringInfo.WeaponClass)
	{
		if (IsValid(Weapon))
		{
			Weapon->OnDetachedFromCharacter();
		}
		Weapon = SpawnWeapon();
	}
	else if (!FiringInfo.bFiring)
	{
		if (IsValid(Weapon))
		{
			Weapon->OnStopFiring();
		}
	}
}

void AStoryModeProjectCharacter::Server_FireWeapon(TSubclassOf<class AWeaponActor> WeaponCls)
{
	if (HasAuthority())
	{
		FiringInfo.bFiring = true;
		FiringInfo.WeaponClass = WeaponCls;

		Weapon = SpawnWeapon();
	}
}

void AStoryModeProjectCharacter::Server_DropWeapon()
{
	if (HasAuthority())
	{
		FiringInfo.bFiring = false;
		FiringInfo.WeaponClass = nullptr;

		Weapon->OnStopFiring();
	}
}

AWeaponActor* AStoryModeProjectCharacter::SpawnWeapon()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;

	const FVector SpawnLocation = GetMuzzlePoint();
	const FRotator SpawnRotation = SpawnLocation.Rotation();

	return GetWorld()->SpawnActor<AWeaponActor>(FiringInfo.WeaponClass, FTransform(SpawnRotation, SpawnLocation), SpawnParameters);
}

void AStoryModeProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStoryModeProjectCharacter, Health);

	DOREPLIFETIME(AStoryModeProjectCharacter, bAiming);
	DOREPLIFETIME(AStoryModeProjectCharacter, RepCameraLocation);
	DOREPLIFETIME(AStoryModeProjectCharacter, RepCameraForwardVector);

	DOREPLIFETIME(AStoryModeProjectCharacter, FiringInfo);
}

UAbilitySystemComponent* AStoryModeProjectCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AStoryModeProjectCharacter::GetHealth() const
{
	return Health;
}

void AStoryModeProjectCharacter::ActivateAbility(int32 InputCode)
{
	if (IsValid(GetAbilitySystemComponent()))
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed(InputCode);
	}
}

void AStoryModeProjectCharacter::DeactivateAbility(int32 InputCode)
{
	if (IsValid(GetAbilitySystemComponent()))
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased(InputCode);
	}
}

void AStoryModeProjectCharacter::CancelAbility(const FGameplayTagContainer CancelWithTags)
{
	GetAbilitySystemComponent()->CancelAbilities(&CancelWithTags);
}


//////////////////////////////////////////////////////////////////////////
// Input

void AStoryModeProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	if (GetLocalRole() < ENetRole::ROLE_AutonomousProxy)
	{
		return;
	}

	/* Example
	{
		FInputActionHandlerSignature ActionHandler;
		ActionHandler.BindUFunction(this, TEXT("OnDash"), EAxis::X, 1.f);
		AddInputActionBinding(ActionHandler, TEXT("DashForward"));
	}*/

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AStoryModeProjectCharacter::OnJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AStoryModeProjectCharacter::OnDash);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AStoryModeProjectCharacter::OnStartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AStoryModeProjectCharacter::OnStopAiming);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AStoryModeProjectCharacter::OnStartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStoryModeProjectCharacter::OnStopFiring);
	
	PlayerInputComponent->BindAction("Shield", IE_Pressed, this, &AStoryModeProjectCharacter::OnStartShielding);
	PlayerInputComponent->BindAction("Shield", IE_Released, this, &AStoryModeProjectCharacter::OnStopShielding);

	PlayerInputComponent->BindAxis("MoveForward", this, &AStoryModeProjectCharacter::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStoryModeProjectCharacter::OnMoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AStoryModeProjectCharacter::OnMoveUp);

	// Don't support anything but PC but let it be here for a while
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AStoryModeProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AStoryModeProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AStoryModeProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AStoryModeProjectCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AStoryModeProjectCharacter::OnResetVR);
}

void AStoryModeProjectCharacter::AddInputActionBinding(FInputActionHandlerSignature& Handler, FName ActionName)
{
	FInputActionBinding ActionBinding = FInputActionBinding(ActionName, IE_Pressed);
	ActionBinding.ActionDelegate = Handler;

	InputComponent->AddActionBinding(ActionBinding);
}

void AStoryModeProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AStoryModeProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AStoryModeProjectCharacter::OnMoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && CanMove())
	{
		if (GetMovementComponent()->IsFlying())
		{
			ApplyFlyMovementInput(EAxis::X, Value);
		}
		else
		{
			ApplyWalkMovementInput(EAxis::X, Value);
		}
	}
}

void AStoryModeProjectCharacter::OnMoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && CanMove())
	{
		if (GetMovementComponent()->IsFlying())
		{
			ApplyFlyMovementInput(EAxis::Y, Value);
		}
		else
		{
			ApplyWalkMovementInput(EAxis::Y, Value);
		}
	}
}

void AStoryModeProjectCharacter::OnMoveUp(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && GetMovementComponent()->IsFlying() && CanMove())
	{
		ApplyFlyMovementInput(EAxis::Z, Value);
	}
}

void AStoryModeProjectCharacter::OnJump()
{
	OnStopAiming();

	Jump();

	UFlyerCharacterMovementComponent* MoveCmp = Cast<UFlyerCharacterMovementComponent>(GetMovementComponent());
	if (MoveCmp)
	{
		if (MoveCmp->IsFalling() || MoveCmp->IsFlying())
		{
			MoveCmp->Fly();
		}
	}
}

void AStoryModeProjectCharacter::ApplyWalkMovementInput(EAxis::Type Axis, float Value)
{
	// find out which way is requested axis
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get axis vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
	// add movement in that direction
	AddMovementInput(Direction, Value);
}

void AStoryModeProjectCharacter::ApplyFlyMovementInput(EAxis::Type Axis, float Value)
{
	const FRotator Rotation = Controller->GetControlRotation();

	const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
}

void AStoryModeProjectCharacter::OnDash()
{
	OnStopAiming();

	UFlyerCharacterMovementComponent* MoveCmp = Cast<UFlyerCharacterMovementComponent>(GetMovementComponent());
	if (MoveCmp)
	{
		MoveCmp->Dash();
	}
}

void AStoryModeProjectCharacter::OnStartAiming()
{
	// "Predict" aiming before server replicates bAiming flag. After replication this will be called again, but in theory this should not be a problem
	bAiming = true;
	OnAimingUpdate();

	Server_Aim(true);
}

void AStoryModeProjectCharacter::OnStopAiming()
{
	// "Predict" aiming before server replicates bAiming flag. After replication this will be called again, but in theory this should not be a problem
	bAiming = false;
	OnAimingUpdate();

	Server_Aim(false);
}

void AStoryModeProjectCharacter::OnStartFiring()
{
	UFlyerCharacterMovementComponent* MoveCmp = Cast<UFlyerCharacterMovementComponent>(GetMovementComponent());
	if (MoveCmp)
	{
		if (MoveCmp->IsFlying())
		{
			if (bAiming)
			{
				// Fire Laser (input code - 0)
				Server_ActivateAbility(0);
			}
			else
			{
				// Fire shrapnel missile (input code - 1)
				Server_ActivateAbility(1);
			}
		}
		else if (MoveCmp->IsWalking())
		{
			if (bAiming)
			{
				// Charge and fire explosive missile
			}
			else
			{
				// Heal up? or some low range rapid attack
			}
		}
		else if (MoveCmp->IsFalling())
		{
			// Come up with something
		}
	}
}

void AStoryModeProjectCharacter::OnStopFiring()
{
	// Drop current WeaponClass so player can wait for the proper update
	//WeaponClass = nullptr;

	/*{
		FGameplayTagContainer Tags;
		if (!bAiming)
		{
			FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Stationary"), false);
			if (Tag.IsValid())
			{
				Tags.AddTag(Tag);
			}
		}

		UFlyerCharacterMovementComponent* MoveCmp = Cast<UFlyerCharacterMovementComponent>(GetMovementComponent());
		if (MoveCmp)
		{
			if (!MoveCmp->IsFlying())
			{
				FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Flying"), false);
				if (Tag.IsValid())
				{
					Tags.AddTag(Tag);
				}
			}
			if (!MoveCmp->IsWalking())
			{
				FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Ground"), false);
				if (Tag.IsValid())
				{
					Tags.AddTag(Tag);
				}
			}
		}
	}*/

	// Stop active firing ability
	Server_CancelAbility(FireTags);
}

void AStoryModeProjectCharacter::OnStartShielding()
{
	// Activate Shield (input code - 2)
	Server_ActivateAbility(2);
}

void AStoryModeProjectCharacter::OnStopShielding()
{
	Server_DeactivateAbility(2);
}

void AStoryModeProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AStoryModeProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AStoryModeProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

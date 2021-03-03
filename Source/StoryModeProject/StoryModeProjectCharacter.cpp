// Copyright Epic Games, Inc. All Rights Reserved.

#include "StoryModeProjectCharacter.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// AStoryModeProjectCharacter

AStoryModeProjectCharacter::AStoryModeProjectCharacter()
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AStoryModeProjectCharacter::BeginPlay()
{
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AStoryModeProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	if (GetLocalRole() >= ENetRole::ROLE_AutonomousProxy)
	{
		{
			FInputActionHandlerSignature ActionHandler;
			ActionHandler.BindUFunction(this, TEXT("OnDash"), EAxis::X, DashVelocity);
			AddInputActionBinding(ActionHandler, TEXT("DashForward"));
		}

		{
			FInputActionHandlerSignature ActionHandler;
			ActionHandler.BindUFunction(this, TEXT("OnDash"), EAxis::X, -DashVelocity);
			AddInputActionBinding(ActionHandler, TEXT("DashBack"));
		}

		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AStoryModeProjectCharacter::OnJump);
		PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

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
}

void AStoryModeProjectCharacter::AddInputActionBinding(FInputActionHandlerSignature& Handler, FName ActionName)
{
	FInputActionBinding ActionBinding = FInputActionBinding(ActionName, IE_Pressed);
	ActionBinding.ActionDelegate = Handler;

	InputComponent->AddActionBinding(ActionBinding);
}

void AStoryModeProjectCharacter::Server_ToggleFlyingMode_Implementation()
{
	UCharacterMovementComponent* CharacterMovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharacterMovementComponent)
	{
		// Check for 2nd time press so we are moved into flying mode
		if (CharacterMovementComponent->IsFalling())
		{
			CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
		}
		// If already if fly mode switch to fall again
		else if (CharacterMovementComponent->IsFlying())
		{
			CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
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
	if ((Controller != NULL) && (Value != 0.0f))
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
	if ((Controller != NULL) && (Value != 0.0f))
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
	if ((Controller != NULL) && (Value != 0.0f) && GetMovementComponent()->IsFlying())
	{
		ApplyFlyMovementInput(EAxis::Z, Value);
	}
}

void AStoryModeProjectCharacter::OnJump()
{
	Jump();

	Server_ToggleFlyingMode();
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

void AStoryModeProjectCharacter::OnDash(EAxis::Type Axis, float Value)
{
	const FRotator Rotation = Controller->GetControlRotation();

	const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	if (!GetMovementComponent()->IsFlying())
	{

	}
	LaunchCharacter(Direction * Value, true, true);
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

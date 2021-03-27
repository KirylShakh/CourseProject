// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "StoryModeProjectCharacter.generated.h"

USTRUCT(BlueprintType, Category = Gameplay)
struct FFiringInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool bFiring = false;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<class AWeaponActor> WeaponClass;
};

/**
 *
 */
UCLASS(config=Game)
class AStoryModeProjectCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* MuzzlePoint;

public:
	AStoryModeProjectCharacter(const class FObjectInitializer& ObjectInitializer);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere)
	class UAbilitySystemComponent* AbilitySystemComponent;

	const class UFlyerAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Abilities)
	TArray<TSubclassOf<class UGameplayAbility>> GameplayAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Abilities)
	FGameplayTagContainer FireTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Abilities)
	FGameplayTagContainer ChargedTags;

protected:

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Called for forwards/backward input */
	void OnMoveForward(float Value);

	/** Called for side to side input */
	void OnMoveRight(float Value);

	/** Called for up/down input */
	void OnMoveUp(float Value);

	/** Manages jumping along with switching to flying mode */
	void OnJump();

	/** Evaluates walking movement input vector when walking by Yaw of Control rotation of the controller */
	void ApplyWalkMovementInput(EAxis::Type Axis, float Value);

	/** Evaluates flying movement input vector when walking by Control rotation of the controller */
	void ApplyFlyMovementInput(EAxis::Type Axis, float Value);

	void OnDash();

	/** Handler for when an aim input begins. */
	void OnStartAiming();

	/** Handler for when an aim input stops. */
	void OnStopAiming();

	/** Handler for when a fire input begins. */
	void OnStartFiring();

	/** Handler for when a fire input stops. */
	void OnStopFiring();

	/** Handler for when a shield input begins. */
	void OnStartShielding();

	/** Handler for when a shield input stops. */
	void OnStopShielding();

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void AddInputActionBinding(FInputActionHandlerSignature& Handler, FName ActionName);

	// Health Stuff
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = Gameplay)
	float Health;

	UFUNCTION()
	virtual void OnRep_Health();

	void OnHealthUpdate();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthUpdated();

	UPROPERTY(BlueprintReadOnly, Category = Gameplay)
	bool bDead = false;

	UFUNCTION(BlueprintImplementableEvent)
	void OnDied();

	// Aiming Stuff
	UPROPERTY(ReplicatedUsing = OnRep_Aiming, BlueprintReadOnly, Category = Gameplay)
	bool bAiming = false;

	UFUNCTION()
	virtual void OnRep_Aiming();

	void OnAimingUpdate();

	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bWantsToAim);

	void Server_Aim_Implementation(bool bWantsToAim);

	bool CanMove();

	// Firing Stuff
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AimRange = 20000.f;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ActivateAbility(int32 InputCode);

	void Server_ActivateAbility_Implementation(int32 InputCode);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DeactivateAbility(int32 InputCode);

	void Server_DeactivateAbility_Implementation(int32 InputCode);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_CancelAbility(const FGameplayTagContainer CancelWithTags);

	void Server_CancelAbility_Implementation(const FGameplayTagContainer CancelWithTags);

	/** Camera location to simulate projectiles for simulated proxies */
	UPROPERTY(Replicated)
	FVector RepCameraLocation;

	/** Camera forward vector to simulate projectiles for simulated proxies */
	UPROPERTY(Replicated)
	FVector RepCameraForwardVector;

	UPROPERTY(ReplicatedUsing = OnRep_Firing, BlueprintReadOnly, Category = Gameplay)
	FFiringInfo FiringInfo;

	UFUNCTION()
	virtual void OnRep_Firing();

	UFUNCTION(BlueprintCallable)
	void Server_FireWeapon(TSubclassOf<class AWeaponActor> WeaponCls);

	UFUNCTION(BlueprintCallable)
	void Server_DropWeapon();

	UPROPERTY(BlueprintReadOnly, Category = Gameplay)
	class AWeaponActor* Weapon;

	UFUNCTION(BlueprintCallable)
	virtual AWeaponActor* SpawnWeapon();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Implement IAbilitySystemInterface
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = Abilities)
	void ActivateAbility(int32 InputCode);

	UFUNCTION(BlueprintCallable, Category = Abilities)
	void DeactivateAbility(int32 InputCode);

	UFUNCTION(BlueprintCallable, Category = Abilities)
	void CancelAbility(const FGameplayTagContainer CancelWithTags);

	UFUNCTION(BlueprintCallable)
	FHitResult GetAimHitResult();

	const FVector GetMuzzlePoint();

	void ShieldDestroyed();
};

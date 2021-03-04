// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FlyerCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class STORYMODEPROJECT_API UFlyerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	friend class FSavedMove_Flyer;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual bool HandlePendingLaunch() override;

	// Dash Staff
	UPROPERTY(EditAnywhere, Category = Dash)
	float DashStrength = 10000.f;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_MoveDirection(const FVector& MoveDir);

	bool Server_MoveDirection_Validate(const FVector& MoveDir);
	void Server_MoveDirection_Implementation(const FVector& MoveDir);

	// Trigger The Dodge Ability. Called from Owning Client
	UFUNCTION(BlueprintCallable, Category = Dash)
	void Dash();

	FVector MoveDirection = FVector::ZeroVector;
	uint8 bWantsToDash : 1;

	// Fly Staff
	UFUNCTION(BlueprintCallable, Category = Fly)
	void Fly();

	uint8 bWantsToFly : 1;

protected:
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;
};

class FSavedMove_Flyer : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	// This is used to check whether or not two moves can be combined into one.
	// Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;
	// Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;
	// Resets all saved variables.
	virtual void Clear() override;
	// Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	// Sets variables on character movement component before making a predictive correction.
	virtual void PrepMoveFor(class ACharacter* Character) override;

private:
	// Dash
	uint8 SavedWantsToDash : 1;
	FVector SavedMoveDirection;
	// Fly
	uint8 SavedWantsToFly : 1;
};

class FNetworkPredictionData_Client_Flyer : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	// Constructor
	FNetworkPredictionData_Client_Flyer(const UCharacterMovementComponent& ClientMovement);

	//brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyerCharacterMovementComponent.h"

#include "GameFramework/Character.h"

FNetworkPredictionData_Client* UFlyerCharacterMovementComponent::GetPredictionData_Client() const
{
    // Should only be called on client in network games
    if (PawnOwner && PawnOwner->GetLocalRole() < ENetRole::ROLE_Authority)
    {
        if (!ClientPredictionData)
        {
            UFlyerCharacterMovementComponent* MutableThis = const_cast<UFlyerCharacterMovementComponent*>(this);

            MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Flyer(*this);
            MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f; // 2X character capsule radius
            MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f; // Numbers are from Unreal Tournament. Investigate?
        }
        return ClientPredictionData;
    }
    return Super::GetPredictionData_Client();
}

void UFlyerCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
    bWantsToFly = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

bool UFlyerCharacterMovementComponent::HandlePendingLaunch()
{
    if (!PendingLaunchVelocity.IsZero() && HasValidData())
    {
        Velocity = PendingLaunchVelocity;
        //SetMovementMode(MOVE_Falling);
        PendingLaunchVelocity = FVector::ZeroVector;
        bForceNextFloorCheck = true;
        return true;
    }

    return false;
}

void UFlyerCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

    if (!CharacterOwner)
    {
        return;
    }

    if (bWantsToDash)
    {
        bWantsToDash = false;

        // Can dash only if on ground or in flying mode
        if (IsMovingOnGround() || IsFlying())
        {
            MoveDirection.Normalize();
            FVector DashVelocity = MoveDirection * DashStrength;
            // If on ground - don't go up as a result of dash
            if (IsMovingOnGround())
            {
                DashVelocity.Z = 0.f;
            }

            Launch(DashVelocity);
        }
    }

    if (bWantsToFly)
    {
        bWantsToFly = false;

        if (IsFalling())
        {
            SetMovementMode(MOVE_Flying);
        }
        else if (IsFlying())
        {
            SetMovementMode(MOVE_Falling);
        }
    }
}

bool UFlyerCharacterMovementComponent::Server_MoveDirection_Validate(const FVector& MoveDir)
{
    return true;
}

void UFlyerCharacterMovementComponent::Server_MoveDirection_Implementation(const FVector& MoveDir)
{
    MoveDirection = MoveDir;
}

void UFlyerCharacterMovementComponent::Dash()
{
    if (PawnOwner->IsLocallyControlled())
    {
        MoveDirection = PawnOwner->GetLastMovementInputVector();
        Server_MoveDirection(MoveDirection);
    }

    bWantsToDash = true;
}

void UFlyerCharacterMovementComponent::Fly()
{
    bWantsToFly = true;
}

bool FSavedMove_Flyer::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
    const FSavedMove_Flyer* NewMove = static_cast<const FSavedMove_Flyer*>(NewMovePtr.Get());

    if (SavedWantsToDash != NewMove->SavedWantsToDash)
    {
        return false;
    }
    if (!SavedMoveDirection.Equals(NewMove->SavedMoveDirection, 0.1f))
    {
        return false;
    }

    if (SavedWantsToFly != NewMove->SavedWantsToFly)
    {
        return false;
    }

    return Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

uint8 FSavedMove_Flyer::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (SavedWantsToDash)
    {
        Result |= FLAG_Custom_0;
    }

    if (SavedWantsToFly)
    {
        Result |= FLAG_Custom_1;
    }

    return Result;
}

void FSavedMove_Flyer::Clear()
{
    Super::Clear();

    SavedWantsToDash = 0;
    SavedMoveDirection = FVector::ZeroVector;

    SavedWantsToFly = 0;
}

void FSavedMove_Flyer::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

    UFlyerCharacterMovementComponent* CharacterMovement = Cast<UFlyerCharacterMovementComponent>(Character->GetMovementComponent());
    if (CharacterMovement)
    {
        SavedWantsToDash = CharacterMovement->bWantsToDash;
        SavedMoveDirection = CharacterMovement->MoveDirection;

        SavedWantsToFly = CharacterMovement->bWantsToFly;
    }
}

void FSavedMove_Flyer::PrepMoveFor(ACharacter* Character)
{
    Super::PrepMoveFor(Character);

    UFlyerCharacterMovementComponent* CharacterMovement = Cast<UFlyerCharacterMovementComponent>(Character->GetMovementComponent());
    if (CharacterMovement)
    {
        CharacterMovement->bWantsToDash = SavedWantsToDash;
        CharacterMovement->MoveDirection = SavedMoveDirection;

        CharacterMovement->bWantsToFly = SavedWantsToFly;
    }
}

FNetworkPredictionData_Client_Flyer::FNetworkPredictionData_Client_Flyer(const UCharacterMovementComponent& ClientMovement) :
    Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Flyer::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_Flyer());
}

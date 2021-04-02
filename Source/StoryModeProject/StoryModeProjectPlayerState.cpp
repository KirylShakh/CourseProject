// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryModeProjectPlayerState.h"

#include "Net/UnrealNetwork.h"

void AStoryModeProjectPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AStoryModeProjectPlayerState, Name);
	DOREPLIFETIME(AStoryModeProjectPlayerState, CharacterClass);
	DOREPLIFETIME(AStoryModeProjectPlayerState, KillCount);
}

void AStoryModeProjectPlayerState::OnRep_Name()
{
	OnUIDataChanged();
}

void AStoryModeProjectPlayerState::OnRep_KillCount()
{
	OnUIDataChanged();
}

void AStoryModeProjectPlayerState::OnRep_CharacterClass()
{
	OnCharacterClassChanged();
}

void AStoryModeProjectPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AStoryModeProjectPlayerState* NewState = Cast<AStoryModeProjectPlayerState>(PlayerState);
	if (NewState)
	{
		CopyInto(NewState);
		NewState->KillCount = KillCount;
	}
}

void AStoryModeProjectPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	AStoryModeProjectPlayerState* NewState = Cast<AStoryModeProjectPlayerState>(PlayerState);
	if (NewState)
	{
		CopyInto(NewState);
	}
}

void AStoryModeProjectPlayerState::CopyInto(AStoryModeProjectPlayerState* PlayerState)
{
	PlayerState->Name = Name;
	PlayerState->Avatar = Avatar;
	PlayerState->CharacterClass = CharacterClass;
}

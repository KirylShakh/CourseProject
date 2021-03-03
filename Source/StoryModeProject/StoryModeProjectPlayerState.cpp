// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryModeProjectPlayerState.h"

#include "Net/UnrealNetwork.h"

void AStoryModeProjectPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Here we list the variables we want to replicate + a condition if wanted
	DOREPLIFETIME(AStoryModeProjectPlayerState, Name);
	DOREPLIFETIME(AStoryModeProjectPlayerState, Avatar);
	DOREPLIFETIME(AStoryModeProjectPlayerState, KillCount);
}

void AStoryModeProjectPlayerState::OnRep_Name()
{
	OnUIDataChanged();
}

void AStoryModeProjectPlayerState::OnRep_Avatar()
{
	OnUIDataChanged();
}

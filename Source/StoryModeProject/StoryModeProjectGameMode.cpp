// Copyright Epic Games, Inc. All Rights Reserved.

#include "StoryModeProjectGameMode.h"
#include "StoryModeProjectCharacter.h"
#include "StoryModeProjectPlayerState.h"

#include "UObject/ConstructorHelpers.h"

AStoryModeProjectGameMode::AStoryModeProjectGameMode()
{
}

void AStoryModeProjectGameMode::PlayerKilled(AStoryModeProjectCharacter* KillerCharacter)
{
	AStoryModeProjectPlayerState* KillerPlayerState = Cast<AStoryModeProjectPlayerState>(KillerCharacter->GetPlayerState());
	if (KillerPlayerState && KillerPlayerState->KillCount >= GoalKillCount)
	{
		OnEndRound(KillerPlayerState);
	}
}

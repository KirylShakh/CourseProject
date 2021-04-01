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
	FPlayerInfo PlayerInfo = KillerCharacter->GetPlayerInfo();
	if (PlayerInfo.KillCount >= GoalKillCount)
	{
		OnEndRound(PlayerInfo);
	}
}

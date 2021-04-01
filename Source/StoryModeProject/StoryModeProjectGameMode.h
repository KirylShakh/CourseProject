// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerInfo.h"
#include "StoryModeProjectGameMode.generated.h"

UCLASS(minimalapi)
class AStoryModeProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStoryModeProjectGameMode();

	void PlayerKilled(class AStoryModeProjectCharacter* KillerCharacter);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerDied(class AStoryModeProjectCharacter* Killer, class AStoryModeProjectCharacter* Victim);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
	int32 RespawnTime = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
	int32 GoalKillCount = 10;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnEndRound(FPlayerInfo Winner);
};




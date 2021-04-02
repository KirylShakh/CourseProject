// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ArenaGameState.generated.h"

/**
 *
 */
UCLASS()
class STORYMODEPROJECT_API AArenaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerList)
	TArray<class AStoryModeProjectCharacter*> Bots;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = PlayerList)
	TArray<class AStoryModeProjectPlayerState*> Players;
};

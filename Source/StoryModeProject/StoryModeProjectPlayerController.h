// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerInfo.h"
#include "StoryModeProjectPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class STORYMODEPROJECT_API AStoryModeProjectPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	TArray<FPlayerInfo> EnemyInfo();
};

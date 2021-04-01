// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FText Name;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Avatar;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount;

	UPROPERTY(BlueprintReadWrite)
	FVector2D ScreenLocation;
};

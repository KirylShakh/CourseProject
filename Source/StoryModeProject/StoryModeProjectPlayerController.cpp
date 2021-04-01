// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryModeProjectPlayerController.h"
#include "StoryModeProjectCharacter.h"
#include "ArenaGameState.h"
#include "StoryModeProjectPlayerState.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

TArray<FPlayerInfo> AStoryModeProjectPlayerController::EnemyInfo()
{
    TArray<FPlayerInfo> Result;

    AArenaGameState* const GameState = GetWorld()->GetGameState<AArenaGameState>();
    if (GameState)
    {
        for (int i = 0; i < GameState->PlayerArray.Num() || i < GameState->Bots.Num(); i++)
        {
            if (i < GameState->PlayerArray.Num())
            {
                AStoryModeProjectCharacter* Enemy = GameState->PlayerArray[i]->GetPawn<AStoryModeProjectCharacter>();
                if (Enemy && Enemy != GetPawn<AStoryModeProjectCharacter>())
                {
                    FPlayerInfo Info = Enemy->GetPlayerInfo();
                    ProjectWorldLocationToScreen(Enemy->GetMuzzlePoint(), Info.ScreenLocation);

                    Result.Add(Info);
                }
            }

            if (i < GameState->Bots.Num())
            {
                AStoryModeProjectCharacter* Enemy = Cast<AStoryModeProjectCharacter>(GameState->Bots[i]);
                if (Enemy && Enemy != GetPawn<AStoryModeProjectCharacter>())
                {
                    FPlayerInfo Info = Enemy->GetPlayerInfo();
                    ProjectWorldLocationToScreen(Enemy->GetMuzzlePoint(), Info.ScreenLocation);

                    Result.Add(Info);
                }
            }
        }
    }

    return Result;
}

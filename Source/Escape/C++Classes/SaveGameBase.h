// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameBase.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API USaveGameBase : public USaveGame
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SaveFloorCount(int NewFloorCount);

	UFUNCTION(BlueprintCallable)
	int GetBestFloorCount();

	UFUNCTION(BlueprintCallable)
	void SaveTime(int NewTime);

	UFUNCTION(BlueprintCallable)
	int GetBestTime();

	bool bPlayedTutorial = false;
private:
	int BestFloorCount = 0;
	int BestTime = 9999999;
};

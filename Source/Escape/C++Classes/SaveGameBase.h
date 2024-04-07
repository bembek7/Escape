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
	void SaveFloorCount(const int32 NewFloorCount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetBestFloorCount() const;

	UFUNCTION(BlueprintCallable)
	void SaveTime(const int32 NewTime);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetBestTime() const;

protected:

private:

public:
	UPROPERTY(BlueprintReadWrite)
	bool bPlayedTutorial;

	static const int32 SaveIndex = 0;
	static const FString SaveSlotName;

protected:

private:
	UPROPERTY()
	int32 BestFloorCount = 0;
	UPROPERTY()
	int32 BestTime = 99999;
};

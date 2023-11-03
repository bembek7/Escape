// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Room.h"
#include "GameFramework/Character.h"

#include "GMBase.generated.h"
/**
 * 
 */
UCLASS()
class ESCAPE_API AGMBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGMBase();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void FloorCompleted();

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void FloorStarted();

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void SetSavePlayedTutorial(bool Played);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rooms")
	int32 GetSavedTime() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rooms")
	int32 GetSavedFloorBeat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rooms")
	FText TimeToText(int32 TimeInHundredthsOfSeconds) const; // Converting time in hundredths of seconds to text in format mm:ss:hh

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rooms")
	FText GetCurrentTimeInText() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rooms")
	int32 GetCurrentFloorBeat() const;

	UFUNCTION(BlueprintCallable)
	void GameStarted();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	uint32 NumberOfRoomsToSpawn = 5;
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	TArray<TSubclassOf<ARoom>>RoomsClasses;
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	TSubclassOf<ARoom>FinalRoomClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple")
	FName GrappleTargetTag = FName("GrappleTarget");

private:
	void SaveScore();
	void SpawnLevel();
	void BindOnDestroyedToPlayer();

	UFUNCTION()
	void PlayerDestroyed(AActor* DestroyedPlayer);

	TSubclassOf<ARoom> GetRandomRoomClass();
	
	uint32 GetRandomRoomIndex(); // gets random room index, randomizing is make that so rooms wouldn't reapet and appear regularly  
	
	void UpdateChances(uint32 ChosenIndex); // updating chances after picking the room to spawn
	void ClearRooms();
	void GenerateEqualChances();
	void SpawnRooms(uint32 RoomsToSpawn, const FTransform& LastExitTransform);
	void SpawnRoom(const TSubclassOf<ARoom>& RoomClass, const FTransform& SpawnTransform);

	uint32 CurrentTime = 0; // Current run time in hundredths of second
	uint32 CurrentFloorBeat = 0; // Current run time in hundredths of second
	uint32 SpawnedLeftTurns = 0;
	uint32 SpawnedRightTurns = 0;
	TArray<uint32>RoomsChancesOfSpawning;
	TArray<ARoom*>SpawnedRooms;
	
	FTransform LastRoomExitTransform;
	
	FTimerHandle FloorTimer;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelExit.h"
#include "Room.generated.h"

UCLASS()
class ESCAPE_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	int32 GetLeftTurns();

	UFUNCTION(BlueprintCallable)
	int32 GetRightTurns();

	FTransform GetExitTransform();

protected:
	UPROPERTY(EditDefaultsOnly)
	uint32 LeftTurns;

	UPROPERTY(EditDefaultsOnly)
	uint32 RightTurns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULevelExit* LevelExit;
private:
	UPROPERTY()
	USceneComponent* DefaultRootComponent;
};

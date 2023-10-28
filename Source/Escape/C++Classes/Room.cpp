// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultRootComponent);
	LevelExit = CreateDefaultSubobject<ULevelExit>(TEXT("LevelExit"));
	LevelExit->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int ARoom::GetLeftTurns()
{
	return LeftTurns;
}

int ARoom::GetRightTurns()
{
	return RightTurns;
}

FTransform ARoom::GetExitTransform()
{
	return LevelExit->GetComponentTransform();
}


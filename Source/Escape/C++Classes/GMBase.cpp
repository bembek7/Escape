// Fill out your copyright notice in the Description page of Project Settings.


#include "GMBase.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameBase.h"

AGMBase::AGMBase()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveIndex))
	{
		USaveGame* SaveObject = UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass());
		UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveIndex);
	}
}

void AGMBase::BeginPlay()
{
	Super::BeginPlay();

	RoomsChancesOfSpawning.Init(0, RoomsClasses.Num());
	GenerateEqualChances();

	LastRoomExitTransform = Cast<ARoom>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoom::StaticClass()))->GetExitTransform();
}

void AGMBase::GenerateEqualChances()
{
	for (int& chance : RoomsChancesOfSpawning)
	{
		chance = 100 / RoomsChancesOfSpawning.Num();
	}
	RoomsChancesOfSpawning.Last() += 100 % RoomsChancesOfSpawning.Num();
}

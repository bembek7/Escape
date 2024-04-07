// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveGameBase.h"

const FString USaveGameBase::SaveSlotName = "DefaultSave";

void USaveGameBase::SaveFloorCount(const int32 NewFloorCount)
{
	BestFloorCount = FMath::Max(BestFloorCount, NewFloorCount);
}

int32 USaveGameBase::GetBestFloorCount() const
{
	return BestFloorCount;
}

void USaveGameBase::SaveTime(const int32 NewTime)
{
	BestTime = FMath::Min(BestTime, NewTime);
}

int32 USaveGameBase::GetBestTime() const
{
	return BestTime;
}
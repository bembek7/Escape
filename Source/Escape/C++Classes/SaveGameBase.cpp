// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameBase.h"

void USaveGameBase::SaveFloorCount(int32 NewFloorCount)
{
	BestFloorCount = FMath::Max(BestFloorCount, NewFloorCount);
}

int32 USaveGameBase::GetBestFloorCount() const
{
	return BestFloorCount;
}

void USaveGameBase::SaveTime(int32 NewTime)
{
	BestTime = FMath::Min(BestTime, NewTime);
}

int32 USaveGameBase::GetBestTime() const
{
	return BestTime;
}

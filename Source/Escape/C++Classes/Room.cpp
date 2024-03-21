#include "Room.h"

ARoom::ARoom() noexcept
{
	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultRootComponent);
	LevelExit = CreateDefaultSubobject<ULevelExit>(TEXT("LevelExit"));
	LevelExit->SetupAttachment(RootComponent);
}

int32 ARoom::GetLeftTurns() const noexcept
{
	return LeftTurns;
}

int32 ARoom::GetRightTurns() const noexcept
{
	return RightTurns;
}

FTransform ARoom::GetExitTransform() const noexcept
{
	return LevelExit->GetComponentTransform();
}
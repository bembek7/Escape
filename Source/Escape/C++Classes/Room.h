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
	ARoom() noexcept;

	UFUNCTION(BlueprintCallable)
	int32 GetLeftTurns() const noexcept;

	UFUNCTION(BlueprintCallable)
	int32 GetRightTurns() const noexcept;

	FTransform GetExitTransform() const noexcept;

protected:

private:

public:

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

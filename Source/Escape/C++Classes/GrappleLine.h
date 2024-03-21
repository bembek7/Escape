// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "GrappleLine.generated.h"

UCLASS()
class ESCAPE_API AGrappleLine : public AActor
{
	GENERATED_BODY()

public:
	AGrappleLine() noexcept;
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void GrappleOn(const FVector& Target) noexcept;
	UFUNCTION(BlueprintCallable)
	void GrappleOff() noexcept;

protected:
	virtual void BeginPlay() override;

private:

public:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCableComponent* GrappleLine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DevelopingSpeed = 4.5f;
private:
	FVector GrappleTarget;
	bool bDeveloping;
	FTimerHandle LineDevelopingTimer;
};

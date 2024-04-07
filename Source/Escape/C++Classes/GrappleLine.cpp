// Fill out your copyright notice in the Description page of Project Settings.

#include "GrappleLine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

AGrappleLine::AGrappleLine()
{
	PrimaryActorTick.bCanEverTick = true;
	GrappleLine = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleLine"));
	GrappleLine->SetVisibility(false);
	SetRootComponent(GrappleLine);
}

void AGrappleLine::BeginPlay()
{
	Super::BeginPlay();
}

void AGrappleLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Developing the grapple from player to a grapple target
	if (bDeveloping && GrappleLine)
	{
		GrappleLine->SetWorldLocation(FMath::VInterpTo(GrappleLine->GetComponentLocation(), GrappleTarget, DeltaTime, DevelopingSpeed));
	}
}

void AGrappleLine::GrappleOn(const FVector& Target)
{
	ACharacter* const PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar && GrappleLine)
	{
		GrappleLine->SetAttachEndToComponent(PlayerChar->GetMesh()); // We are attaching end of grapple line to player
		GrappleTarget = Target;
		GrappleLine->SetVisibility(true);
		bDeveloping = true;
		const float LineDevelopingInRate = 0.5f;
		GetWorldTimerManager().SetTimer(LineDevelopingTimer, [this]() {bDeveloping = false; }, LineDevelopingInRate, false);
	}
}

void AGrappleLine::GrappleOff() noexcept
{
	bDeveloping = false;
	if(GrappleLine)
	{
		GrappleLine->SetVisibility(false);
	}
}
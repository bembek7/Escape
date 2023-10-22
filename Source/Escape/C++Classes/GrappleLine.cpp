// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleLine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AGrappleLine::AGrappleLine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GrappleLine = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleLine"));
	GrappleLine->SetVisibility(false);
}

// Called when the game starts or when spawned
void AGrappleLine::BeginPlay()
{
	Super::BeginPlay();
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	GrappleLine->SetAttachEndToComponent(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetMesh());
}

// Called every frame
void AGrappleLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bDeveloping)GrappleLine->SetWorldLocation(FMath::VInterpTo(GrappleLine->GetComponentLocation(), GrappleTarget, DeltaTime, DevelopingSpeed));
}

void AGrappleLine::GrappleOn(const FVector& Target)
{
	GrappleTarget = Target;
	GrappleLine->SetVisibility(true);
	bDeveloping = true;
	float DelayTime = 0.5f;
	GetWorldTimerManager().SetTimer(LineDevelopingTimer, [this]() {bDeveloping = false; }, 0.5f, false);
}

void AGrappleLine::GrappleOff()
{
	bDeveloping = false;
	GrappleLine->SetVisibility(false);
}


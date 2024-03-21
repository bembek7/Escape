// Fill out your copyright notice in the Description page of Project Settings.

#include "W_HUD.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UW_HUD::NativeConstruct()
{
	Super::NativeConstruct();
	if (DashIcon) // binding the material to image
	{
		DashIconMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), IconMaterialParent);
		DashIcon->SetBrushFromMaterial(DashIconMaterial);
	}
}

void UW_HUD::UpdateDashIconScan(const float Percent) noexcept // scanning the dash icon
{
	if (DashIconMaterial)
	{
		DashIconMaterial->SetScalarParameterValue(ScalarParameterName, Percent);
	}
}
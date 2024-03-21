// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"

#include "W_HUD.generated.h"

/**
 *
 */
UCLASS()
class ESCAPE_API UW_HUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateDashIconScan(const float Percent) noexcept;

protected:

private:

public:

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UImage* DashIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* IconMaterialParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* DashIconMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ScalarParameterName = FName("Percent");

private:
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "W_HUD.h"
#include "PlayerControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void PauseUnpause();

	void UpdateDashIconScanHudWidget(float Percent);
	
	UFUNCTION(BlueprintCallable)
	void TeleportToTutorial();
	void TeleportToSpawn() const;

	FVector GetPlayerSpawnLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideMainMenu();
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideDeathWidget();

	void ShowTimeWidget();
	void HideTimeWidget();
	void ShowFloorCompletedWidget();
	void ShowDeathWidget();

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideFloorCompletedWidget();

	UPROPERTY(BLueprintReadWrite)
	bool bInTutorial;

protected: 

	// Widgets, blueprint children will set the widgets classes

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UW_HUD>HudWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>PauseWidgetClass; // blueprint child will set the widget class

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>MainMenuClass; // blueprint child will set the widget class

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>DeathWidgetClass; // blueprint child will set the widget class

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>TimeWidgetClass; // blueprint child will set the widget class

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>FloorCompletedWidgetClass; // blueprint child will set the widget class

private:
	void DecideSpawnLocation();

	// Widgets

	void CreateWidgets();
	void ShowWidgetToFocus(UUserWidget* WidgetToShow);
	void HideFocusedWidget(UUserWidget* WidgetToHide);
	void ShowWidgetAndPause(UUserWidget* WidgetToShow);
	void HideWidgetAndUnpause(UUserWidget* WidgetToHide);


	FVector PlayerSpawnLocation;
	FVector TutorialStartLocation;
	FVector RegularStartLocation;

	// Widgets

	UPROPERTY()
	UW_HUD* HudWidget;
	UPROPERTY()
	UUserWidget* PauseWidget;
	UPROPERTY()
	UUserWidget* MainMenu;
	UPROPERTY()
	UUserWidget* DeathWidget;
	UPROPERTY()
	UUserWidget* FloorCompletedWidget;
	UPROPERTY()
	UUserWidget* TimeWidget;
};

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

	UFUNCTION(Category = "Widgets")
	void UpdateDashIconScanHudWidget(float Percent);

	UFUNCTION(BlueprintCallable)
	void TeleportToTutorial();

	UFUNCTION(BlueprintCallable)
	void HideMainMenu();

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
	UFUNCTION()
	void DecideSpawnLocation();

	// Widgets

	UFUNCTION(Category = "Widgets")
	void CreateWidgets();

	UFUNCTION(Category = "Widgets")
	void ShowWidgetToFocus(UUserWidget* WidgetToShow);

	UFUNCTION(Category = "Widgets")
	void HideFocusedWidget(UUserWidget* WidgetToHide);

	UFUNCTION(Category = "Widgets")
	void ShowWidgetAndPause(UUserWidget* WidgetToShow);

	UFUNCTION(Category = "Widgets")
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

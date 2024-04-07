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

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void PauseUnpause();

	void UpdateDashIconScanHudWidget(const float Percent);

	UFUNCTION(BlueprintCallable)
	void TeleportToTutorial();
	UFUNCTION(BlueprintCallable)
	void SetSpawnLocationToRegular();
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

protected:
	virtual void BeginPlay() override;

private:
	void CreateWidgets();
	void ShowWidgetToFocus(UUserWidget* WidgetToShow);
	void HideFocusedWidget(UUserWidget* WidgetToHide);
	void ShowWidgetAndPause(UUserWidget* WidgetToShow);
	void HideWidgetAndUnpause(UUserWidget* WidgetToHide);
	void DecideSpawnLocation();

public:
	UPROPERTY(BLueprintReadWrite)
	bool bInTutorial;

protected:
	// Widgets, blueprint children will set the widgets classes
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UW_HUD>HudWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>DeathWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>TimeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget>FloorCompletedWidgetClass;

private:
	FVector PlayerSpawnLocation;
	FVector TutorialStartLocation;
	FVector RegularStartLocation;

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

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
	void PauseUnpause() noexcept;

	void UpdateDashIconScanHudWidget(const float Percent) noexcept;

	UFUNCTION(BlueprintCallable)
	void TeleportToTutorial() noexcept;
	UFUNCTION(BlueprintCallable)
	void SetSpawnLocationToRegular() noexcept;
	void TeleportToSpawn() const noexcept;
	FVector GetPlayerSpawnLocation() const noexcept;

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideMainMenu() noexcept;
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideDeathWidget() noexcept;
	void ShowTimeWidget() noexcept;
	void HideTimeWidget() noexcept;
	void ShowFloorCompletedWidget()  noexcept;
	void ShowDeathWidget() noexcept;
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideFloorCompletedWidget() noexcept;

protected:
	virtual void BeginPlay() override;

private:
	void CreateWidgets() noexcept;
	void ShowWidgetToFocus(UUserWidget* WidgetToShow) noexcept;
	void HideFocusedWidget(UUserWidget* WidgetToHide) noexcept;
	void ShowWidgetAndPause(UUserWidget* WidgetToShow) noexcept;
	void HideWidgetAndUnpause(UUserWidget* WidgetToHide) noexcept;
	void DecideSpawnLocation() noexcept;

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

#include "PlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "RegularStart.h"
#include "GMBase.h"
#include "GameFramework/PlayerStart.h"
#include "SaveGameBase.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* TutorialStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
	{
		TutorialStartLocation = TutorialStart->GetActorLocation();
	}
	if (AActor* RegularStart = UGameplayStatics::GetActorOfClass(GetWorld(), ARegularStart::StaticClass()))
	{
		RegularStartLocation = RegularStart->GetActorLocation();
	}
	// Deciding Spawn Location and teleporting player there
	DecideSpawnLocation();
	TeleportToSpawn();
	CreateWidgets();
	HudWidget->AddToPlayerScreen();
	ShowWidgetToFocus(MainMenu);
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::DecideSpawnLocation() noexcept
{
	USaveGameBase* SaveObject;
	if (!UGameplayStatics::DoesSaveGameExist(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex))
	{
		SaveObject = Cast<USaveGameBase>(UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass()));
		UGameplayStatics::SaveGameToSlot(SaveObject, USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex);
	}
	else
	{
		SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex));
	}
	const bool bPlayedTutorial = SaveObject->bPlayedTutorial;
	if (bPlayedTutorial)
	{
		PlayerSpawnLocation = RegularStartLocation;
	}
	else
	{
		PlayerSpawnLocation = TutorialStartLocation;
		bInTutorial = true;
	}
}

void APlayerControllerBase::CreateWidgets() noexcept
{
	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UW_HUD>(this, HudWidgetClass);
	}

	if (PauseWidgetClass)
	{
		PauseWidget = CreateWidget<UUserWidget>(this, PauseWidgetClass);
	}

	if (MainMenuClass)
	{
		MainMenu = CreateWidget<UUserWidget>(this, MainMenuClass);
	}

	if (DeathWidgetClass)
	{
		DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass);
	}

	if (FloorCompletedWidgetClass)
	{
		FloorCompletedWidget = CreateWidget<UUserWidget>(this, FloorCompletedWidgetClass);
	}

	if (TimeWidgetClass)
	{
		TimeWidget = CreateWidget<UUserWidget>(this, TimeWidgetClass);
	}
}

void APlayerControllerBase::ShowWidgetToFocus(UUserWidget* WidgetToShow) noexcept
{
	WidgetToShow->AddToPlayerScreen();
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void APlayerControllerBase::HideFocusedWidget(UUserWidget* WidgetToHide) noexcept
{
	WidgetToHide->RemoveFromParent();
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void APlayerControllerBase::ShowWidgetAndPause(UUserWidget* WidgetToShow) noexcept
{
	ShowWidgetToFocus(WidgetToShow);
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void APlayerControllerBase::HideWidgetAndUnpause(UUserWidget* WidgetToHide) noexcept
{
	HideFocusedWidget(WidgetToHide);
	UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void APlayerControllerBase::PauseUnpause() noexcept
{
	if (PauseWidget)
	{
		if (!PauseWidget->IsVisible())
		{
			ShowWidgetAndPause(PauseWidget);
		}
		else
		{
			HideWidgetAndUnpause(PauseWidget);
		}
	}
}

void APlayerControllerBase::UpdateDashIconScanHudWidget(const float Percent) noexcept
{
	HudWidget->UpdateDashIconScan(Percent);
}

void APlayerControllerBase::TeleportToTutorial() noexcept
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(TutorialStartLocation);
	PlayerSpawnLocation = TutorialStartLocation;
	bInTutorial = true;
}

void APlayerControllerBase::SetSpawnLocationToRegular() noexcept
{
	PlayerSpawnLocation = RegularStartLocation;
}

void APlayerControllerBase::TeleportToSpawn() const noexcept
{
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(PlayerSpawnLocation);
}

FVector APlayerControllerBase::GetPlayerSpawnLocation() const noexcept
{
	return PlayerSpawnLocation;
}

void APlayerControllerBase::HideMainMenu() noexcept
{
	HideFocusedWidget(MainMenu);
}

void APlayerControllerBase::HideDeathWidget() noexcept
{
	HideWidgetAndUnpause(DeathWidget);
}

void APlayerControllerBase::ShowTimeWidget() noexcept
{
	TimeWidget->AddToPlayerScreen();
}

void APlayerControllerBase::HideTimeWidget() noexcept
{
	TimeWidget->RemoveFromParent();
}

void APlayerControllerBase::ShowFloorCompletedWidget() noexcept
{
	ShowWidgetAndPause(FloorCompletedWidget);
}

void APlayerControllerBase::ShowDeathWidget() noexcept
{
	ShowWidgetAndPause(DeathWidget);
}

void APlayerControllerBase::HideFloorCompletedWidget() noexcept
{
	HideWidgetAndUnpause(FloorCompletedWidget);
}
#include "PlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "RegularStart.h"
#include "GMBase.h"
#include "GameFramework/PlayerStart.h"
#include "SaveGameBase.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* const TutorialStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
	{
		TutorialStartLocation = TutorialStart->GetActorLocation();
	}
	if (AActor* const RegularStart = UGameplayStatics::GetActorOfClass(GetWorld(), ARegularStart::StaticClass()))
	{
		RegularStartLocation = RegularStart->GetActorLocation();
	}
	// Deciding Spawn Location and teleporting player there
	DecideSpawnLocation();
	TeleportToSpawn();
	CreateWidgets();
	if (HudWidget)
	{
		HudWidget->AddToPlayerScreen();
	}
	if (MainMenu)
	{
		ShowWidgetToFocus(MainMenu);
	}
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::DecideSpawnLocation()
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

void APlayerControllerBase::CreateWidgets()
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

void APlayerControllerBase::ShowWidgetToFocus(UUserWidget* const WidgetToShow)
{
	if (WidgetToShow)
	{
		WidgetToShow->AddToPlayerScreen();
	}
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void APlayerControllerBase::HideFocusedWidget(UUserWidget* const WidgetToHide)
{
	if (WidgetToHide)
	{
		WidgetToHide->RemoveFromParent();
	}
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void APlayerControllerBase::ShowWidgetAndPause(UUserWidget* const WidgetToShow)
{
	ShowWidgetToFocus(WidgetToShow);
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void APlayerControllerBase::HideWidgetAndUnpause(UUserWidget* const WidgetToHide)
{
	HideFocusedWidget(WidgetToHide);
	UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void APlayerControllerBase::PauseUnpause()
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

void APlayerControllerBase::UpdateDashIconScanHudWidget(const float Percent)
{
	if (HudWidget)
	{
		HudWidget->UpdateDashIconScan(Percent);
	}
}

void APlayerControllerBase::TeleportToTutorial()
{
	if (ACharacter* const PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		PlayerChar->SetActorLocation(TutorialStartLocation);
	}
	PlayerSpawnLocation = TutorialStartLocation;
	bInTutorial = true;
}

void APlayerControllerBase::SetSpawnLocationToRegular()
{
	PlayerSpawnLocation = RegularStartLocation;
}

void APlayerControllerBase::TeleportToSpawn() const
{
	if (ACharacter* const PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		PlayerChar->SetActorLocation(PlayerSpawnLocation);
	}
}

FVector APlayerControllerBase::GetPlayerSpawnLocation() const
{
	return PlayerSpawnLocation;
}

void APlayerControllerBase::HideMainMenu()
{
	HideFocusedWidget(MainMenu);
}

void APlayerControllerBase::HideDeathWidget()
{
	HideWidgetAndUnpause(DeathWidget);
}

void APlayerControllerBase::ShowTimeWidget()
{
	TimeWidget->AddToPlayerScreen();
}

void APlayerControllerBase::HideTimeWidget()
{
	TimeWidget->RemoveFromParent();
}

void APlayerControllerBase::ShowFloorCompletedWidget()
{
	ShowWidgetAndPause(FloorCompletedWidget);
}

void APlayerControllerBase::ShowDeathWidget()
{
	ShowWidgetAndPause(DeathWidget);
}

void APlayerControllerBase::HideFloorCompletedWidget()
{
	HideWidgetAndUnpause(FloorCompletedWidget);
}
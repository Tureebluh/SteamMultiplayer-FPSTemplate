// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MainMenu.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CircularThrobber.h"

#include "Game/SessionBtn.h"


UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> SessionBtnBPClass(TEXT("/Game/UI/WBP_SessionBtn"));
	if (!ensure(SessionBtnBPClass.Class != nullptr)) return;

	SessionBtnClass = SessionBtnBPClass.Class;
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("MainMenu failed to initialize.")); return false; }

	if (!ensure(MainHostBtn != nullptr)) { return false; }
	MainHostBtn->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);
	if (!ensure(MainJoinBtn != nullptr)) { return false; }
	MainJoinBtn->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);
	if (!ensure(MainQuitBtn != nullptr)) { return false; }
	MainQuitBtn->OnClicked.AddDynamic(this, &UMainMenu::QuitGamePressed);

	if (!ensure(HostBackBtn != nullptr)) { return false; }
	HostBackBtn->OnClicked.AddDynamic(this, &UMainMenu::BackToMainMenu);

	if (!ensure(JoinConfirmBtn != nullptr)) { return false; }
	JoinConfirmBtn->OnClicked.AddDynamic(this, &UMainMenu::JoinAGame);
	if (!ensure(JoinBackBtn != nullptr)) { return false; }
	JoinBackBtn->OnClicked.AddDynamic(this, &UMainMenu::BackToMainMenu);

	return true;
}

void UMainMenu::SetMenuInterface(IMenuInterface* Interface)
{
	MenuInterface = Interface;
}

void UMainMenu::Setup()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return; }

	this->AddToViewport();
	FInputModeGameAndUI InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	World->GetFirstPlayerController()->SetInputMode(InputModeData);
	World->GetFirstPlayerController()->bShowMouseCursor = true;
}

void UMainMenu::OpenHostMenu()
{
	if (!ensure(MainMenuSwitcher != nullptr)) { return; }
	MainMenuSwitcher->SetActiveWidgetIndex(1);
}

void UMainMenu::OpenJoinMenu()
{
	if (!ensure(MainMenuSwitcher != nullptr)) { return; }
	MainMenuSwitcher->SetActiveWidgetIndex(2);
	if (!ensure(MenuInterface != nullptr)) { return; }
	MenuInterface->RefreshServerList();
}

void UMainMenu::SetServerList(TArray<FString> SessionNames)
{
	if (!ensure(SessionList != nullptr)) { return; }

	for (FString Session : SessionNames)
	{
		USessionBtn* SessionBtn = CreateWidget<USessionBtn>(this, SessionBtnClass);
		if (!ensure(SessionBtn != nullptr)) { return; }
		SessionBtn->SetSessionName(FText::FromString(Session));
		SessionList->AddChild(SessionBtn);
	}
	LoadingThrobber->SetVisibility(ESlateVisibility::Collapsed);
}

void UMainMenu::ClearServerList()
{
	if (!ensure(SessionList != nullptr)) { return; }
	SessionList->ClearChildren();
	LoadingThrobber->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenu::JoinAGame()
{
	if (!ensure(MenuInterface != nullptr)) { return; }
	MenuInterface->JoinMap("");
}

void UMainMenu::BackToMainMenu()
{
	if (!ensure(MainMenuSwitcher != nullptr)) { return; }
	MainMenuSwitcher->SetActiveWidgetIndex(0);
}

void UMainMenu::QuitGamePressed()
{
	if (!ensure(MenuInterface != nullptr)) { return; }
	MenuInterface->QuitGame();
}
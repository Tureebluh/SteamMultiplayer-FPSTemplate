// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MainMenu.generated.h"

/**
 * 
 */

UCLASS()
class HEISTFPS_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);

	void SetServerList(TArray<FString> SessionNames);

	void ClearServerList();

	void SetMenuInterface(IMenuInterface* Interface);

	void Setup();

protected:
	virtual bool Initialize();


private:

	TSubclassOf<class UUserWidget> SessionBtnClass;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainHostBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainJoinBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainQuitBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostBackBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinConfirmBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinBackBtn;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MainMenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* SessionList;

	UPROPERTY(meta = (BindWidget))
	class UCircularThrobber* LoadingThrobber;

	UFUNCTION()
	void OpenHostMenu();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void JoinAGame();

	UFUNCTION()
	void BackToMainMenu();

	UFUNCTION()
	void QuitGamePressed();

	IMenuInterface* MenuInterface;
};

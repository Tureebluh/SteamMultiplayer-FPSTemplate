// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/MenuInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HeistFPSGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FMapInfo {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText MapURL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText MapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText MapDescription;
};

UCLASS()
class HEISTFPS_API UHeistFPSGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()

public:

	UHeistFPSGameInstance(const FObjectInitializer &ObjectInitializer);

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadMainMenu();

	void TogglePauseMenu();

protected:
	UFUNCTION()
	void HostMap() override;
	UFUNCTION()
	void JoinMap(uint32 SessionIndex) override;
	UFUNCTION()
	void QuitGame() override;
	UFUNCTION()
	void RefreshServerList() override;

private:
	TSubclassOf<class UUserWidget> PauseMenuClass;
	class UUserWidget* PauseMenu;

	TSubclassOf<class UUserWidget> MainMenuClass;
	class UMainMenu* MainMenu;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void CreateSession();
};

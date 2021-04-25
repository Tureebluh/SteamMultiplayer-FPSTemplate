// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/MenuInterface.h"
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

	void TogglePauseMenu();

protected:
	
	void HostMap(FText MapURL) override;
	
	void JoinMap(const FString& IpAddress) override;

private:
	TSubclassOf<class UUserWidget> PauseMenuClass;
	class UUserWidget* PauseMenu;
};

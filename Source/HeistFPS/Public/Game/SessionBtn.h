// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionBtn.generated.h"

/**
 * 
 */
UCLASS()
class HEISTFPS_API USessionBtn : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSessionName(FText SessionNameText);
	
	void Setup(class UMainMenu* InParent, uint32 InIndex);


private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SessionName;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinSessionBtn;

	UPROPERTY()
	class UMainMenu* Parent;

	uint32 Index;

	UFUNCTION()
	void OnClicked();
	
};

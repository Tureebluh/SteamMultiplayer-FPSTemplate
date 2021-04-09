// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
class HEISTFPS_API UHeistFPSGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void LoadMap(FText MapURL);
	
};
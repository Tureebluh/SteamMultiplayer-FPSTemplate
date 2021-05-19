// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapBtn.generated.h"

/**
 * 
 */
UCLASS()
class HEISTFPS_API UMapBtn : public UUserWidget
{
	GENERATED_BODY()

public:
	UMapBtn(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool Initialize();
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UMenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HEISTFPS_API IMenuInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable)
	virtual void HostMap(FText MapURL) = 0;

	UFUNCTION(BlueprintCallable)
	virtual void JoinMap(const FString& IpAddress) = 0;

	UFUNCTION(BlueprintCallable)
	virtual void QuitGame() = 0;

	UFUNCTION(BlueprintCallable)
	virtual void RefreshServerList() = 0;

};

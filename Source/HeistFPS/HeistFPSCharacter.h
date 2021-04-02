// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeistFPSCharacter.generated.h"

UCLASS(config=Game)
class AHeistFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHeistFPSCharacter();

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPSCamera;

	/** Actor Component for spawning and destroying guns on equip */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	class UChildActorComponent* Weapon;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	float MaxWalkSpeed = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	float MaxCrouchSpeed = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	bool bAimOffsetRotation;

	/** Initiates combat for animations */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bCombatInitiated = false;

	/** Used for rifle animations */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bRifleEquipped = false;

	/** Used for ADS rifle animations */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bAimDownSight = false;

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFPSCamera() const { return FPSCamera; }

protected:

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void BeginPlay();

	void Tick(float DeltaTime);

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void ToggleCrouch();

	void Sprint();

	void StopSprint();

	void EquipRifle();

	void AimDownSight();

	void UpdateCharacterAnimMovement(float DeltaTime);
	
};


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

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	float MaxWalkSpeed = 250.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	float MaxSprintSpeed = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	float MaxCrouchSpeed = 150.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float AutoRotationThreshold = 45.0f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentSpeed;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentDirection;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentPitch;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	bool bAimOffsetRotation = false;

	UPROPERTY(ReplicatedUsing=OnRep_bIsSprinting, VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsSprinting = false;

	/** Initiates combat for animations */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bCombatInitiated = false;

	/** Used for primary weapon animations */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bPrimaryEquipped = false;

	/** Used for ADS animations */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bAimDownSight = false;

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFPSCamera() const { return FPSCamera; }

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void BeginPlay();

	void Tick(float DeltaTime);

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void ToggleCrouch();

	void ToggleSprint();

	void EquipPrimaryWeapon();

	void AimDownSight();

	void UpdateCharacterAnimMovement(float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAimDownSight();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipPrimaryWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerToggleSprint();

	UFUNCTION()
	void OnRep_bIsSprinting();

};


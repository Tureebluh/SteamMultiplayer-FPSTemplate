// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class HEISTFPS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Handle cosmetic aspects of weapon firing
	virtual void SimulateWeaponFire();

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetADSCamera() const { return ADSCamera; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Name for weapon firing FX spawn point
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	// Name for ADSCamera spawn point
	UPROPERTY(EditDefaultsOnly, Category = Aiming)
	FName ADSCameraAttachPoint;

	// FX for weapon firing
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	class UNiagaraSystem* MuzzleFX;

	// Component for weapon FX
	UPROPERTY(Transient)
	class UNiagaraComponent* MuzzlePSC;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* ADSCamera;
	

private:	
	// Skeletal mesh for weapon
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* WeaponMesh;
};

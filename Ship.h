// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "BaseHUD.h"
#include "ShipData.h"
#include "ShipController.h"
#include "Ship.generated.h"

class ABaseProjectile;
class UWeapon;
class UAbility;

UCLASS(Blueprintable)
class AShip : public ACharacter
{
	GENERATED_BODY()

		/* The mesh component */
		UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	USphereComponent* aoeDecetion;

public:
	AShip();
	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface

	/* Fire a shot in the specified direction */
	void FireShot(FVector FireDirection);

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(class AController* NewController)override;
	void CalculateDamage(float damage);

	FTimerHandle TimerHandle_CanRegenShieldsExpired;

	FTimerHandle TimerHandle_CanRegenEnergyExpired;

	USphereComponent* GetAoeDetection();
	UFUNCTION()
		void ActivateAbility();

		UFUNCTION(Client, Reliable, BlueprintCallable, Category = "stats")
		void UpdateStats();

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetHealth();

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetShield();

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetEnergy();

	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	void HitByProjectile(ABaseProjectile* damager);

	void OnRep_EnergyChanged();
	void OnRep_HealthChanged();
	void OnRep_ShieldsChanged();

	UFUNCTION()
		void WeaponOne();

	UFUNCTION()
		void WeaponThree();

	UFUNCTION()
		void WeaponTwo();

	UFUNCTION()
		void SetEnergy(float newVal);

	UFUNCTION()
		void SetHealth(float newVal);

	UFUNCTION()
		void SetShield(float newVal);

	UFUNCTION()
		float GetMaxEnergy();

	UFUNCTION()
		float GetMaxHealth();

	UFUNCTION()
		float GetMaxShield();

	UFUNCTION()
		void SetMaxEnergy(float newVal);

	UFUNCTION()
		void SetMaxHealth(float newVal);

	UFUNCTION()
		void SetMaxShield(float newVal);


	UFUNCTION()
		void ResetCanFire();

	UFUNCTION()
		float GetEnergyRegen();

	UFUNCTION()
		float GetShieldRegen();

	UFUNCTION()
		void StartFire();

	UFUNCTION()
		void StopFire();

	UFUNCTION()
		void UpdateRotation();

	UFUNCTION()
		void SetSpeed(float newSpeed);

	FShipData getOriginalData() const { return originalData; }
	FShipData getCurrentData() const { return currentData; }

	float GetMoveSpeed() const { return MoveSpeed; }
	void SetMoveSpeed(float val) { MoveSpeed = val; }
private:
	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;
	
	/* The speed our ship moves around the level */
	UPROPERTY(Replicated, Category = Gameplay, EditAnywhere)
		float MoveSpeed;

	UPROPERTY()
		ABaseHUD* hud;

	UPROPERTY()
		bool firing;

	UPROPERTY(Replicated, EditAnywhere,  Category = "Stats")
		FShipData originalData;

	UPROPERTY(Replicated, EditAnywhere,  Category = "Stats")
		FShipData currentData;
	
	UPROPERTY()
		bool canRegenShields;

	UPROPERTY()
		bool canRegenEnergy;

	UPROPERTY()
		TArray<UWeapon*> weapons;

	UPROPERTY()
		TArray<UAbility*> abilities;

	UPROPERTY()
		UWeapon* currentWeapon;
/*
	UPROPERTY()
		UMovementComponent* movement;*/

	UPROPERTY()
		AShipController* shipController;

	UFUNCTION()
		void Regenerate(float DeltaSeconds);

	UFUNCTION()
		void ResetCanRegenShields();

	UFUNCTION()
		void ResetCanRegenEnergy();

	UFUNCTION()
		void Move(float DeltaSeconds);
	};
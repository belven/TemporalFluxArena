// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "BaseHUD.h"
#include "Ship.generated.h"

class ABaseProjectile;
class UWeapon;
class UAbility;

UCLASS(Blueprintable)
class AShip : public APawn
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
	UPROPERTY(replicated)
		float health;

	UPROPERTY(replicated)
		float shield;

	UPROPERTY(replicated)
		float energy;

	UPROPERTY(ReplicatedUsing = OnRep_LocationChanged, replicated)
		FVector location;

	UPROPERTY(ReplicatedUsing = OnRep_RotationChanged, replicated)
		FRotator rotation;

	UFUNCTION()
		void OnRep_LocationChanged();

	UFUNCTION()
		void OnRep_RotationChanged();

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MoveSpeed;

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

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetHealth();

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetShield();

	UFUNCTION(BlueprintCallable, Category = "Stats")
		float GetEnergy();

private:

	float maxHealth;
	float maxShield;
	float maxEnergy;

	float shieldRegen;
	float energyRegen;

	bool canRegenShields;
	bool canRegenEnergy;
	
	TArray<UWeapon*> weapons;
	TArray<UAbility*> abilities;

	UWeapon* currentWeapon;

	UMovementComponent* movement;

	void Regenerate(float DeltaSeconds);

	void ResetCanRegenShields();

	void ResetCanRegenEnergy();

	void Move(float DeltaSeconds);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	ABaseHUD* hud;
public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	void HitByProjectile(ABaseProjectile* damager);
	USphereComponent* GetAoeDetection();

	void ActivateAbility();
};

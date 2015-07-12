// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "ShipAttachment.h"
#include "ProjectileData.h"
#include "Weapon.generated.h"


class ABaseProjectile;
/**
 *
 */
UCLASS()
class TEMPORALFLUXARENA_API UWeapon : public UShipAttachment
{
	GENERATED_BODY()

public:
	UWeapon();

	bool Fire(class AShip* owner, FVector FireDirection);

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* fireSound;

	static UWeapon* CreateWeapon(AActor* other, int ammo, int fireRate, int ammoRechargeRate, FVector offset);

	int GetAmmo();

	float GetAmmoRechargeRate();

	float GetFireRate();

	void SetFireRate(float newRate);

	bool CanFire();

	void ResetCanFire();

	void Fired(UWorld* const World);
	FProjectileData GetProjectileData();
	void SetProjectileData(FProjectileData newVal);

	bool bCanFire;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;
private:
	int ammo;
	float fireRate;
	float ammoRechargeRate;
	FProjectileData projectileData;
};

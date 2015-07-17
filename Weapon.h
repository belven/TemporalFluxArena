// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "ShipAttachment.h"
#include "ProjectileData.h"
#include "Weapon.generated.h"

UCLASS()
class TEMPORALFLUXARENA_API UWeapon : public UShipAttachment
{
	GENERATED_BODY()

public:
	UWeapon();

	bool Fire(class AShip* owner, FVector FireDirection);

	static UWeapon* CreateWeapon(AActor* other, float fireRate, FVector offset, FProjectileData data);


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float weaponFireRate;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* fireSound;

	void SetFireRate(float newRate);

	bool CanFire();
	bool bCanFire;
	void ResetCanFire();
	float GetFireRate();

	void Fired(UWorld* const World);
	void SetProjectileData(FProjectileData newVal);

	FProjectileData GetProjectileData();

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;
private:
	FProjectileData projectileData;
};

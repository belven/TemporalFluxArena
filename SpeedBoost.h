// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Ability.h"
#include "SpeedBoost.generated.h"

/**
 * 
 */
UCLASS()
class TEMPORALFLUXARENA_API USpeedBoost : public UAbility
{
	GENERATED_BODY()

public:
	virtual void Activate(class AShip* abilityOwner) override;

	static USpeedBoost* CreateAbility();

	void ResetMovementSpeed();




	FTimerHandle TimerHandle_BoostExpired;
	
};

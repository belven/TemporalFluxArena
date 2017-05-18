// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Ability.h"
#include "Teleport.generated.h"

/**
 *
 */
UCLASS()
class TEMPORALFLUXARENA_API UTeleport : public UAbility
{
	GENERATED_BODY()

public:
	virtual void Activate(class AShip* abilityOwner) override;

	static UTeleport* CreateAbility();
};

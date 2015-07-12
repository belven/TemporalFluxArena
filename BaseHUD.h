// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

/**
 *
 */
UCLASS()
class TEMPORALFLUXARENA_API ABaseHUD : public AHUD
{
	GENERATED_BODY()

public:

	//virtual void DrawHUD() override;

	UPROPERTY(BlueprintReadWrite, Category = "Stats")
		float health;

	UPROPERTY(BlueprintReadWrite, Category = "Stats")
		float shield;

	UPROPERTY(BlueprintReadWrite, Category = "Stats")
		float energy;
};

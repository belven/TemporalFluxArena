// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "ShipAttachment.generated.h"

UCLASS()
class TEMPORALFLUXARENA_API UShipAttachment : public UObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UShipAttachment();

	FVector GetOffset();

	FVector offset;	
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "SpeedBoost.h"
#include "Ship.h"

void USpeedBoost::Activate(AShip* owner){
	this->owner = owner;
	owner->SetSpeed(owner->MoveSpeed * 1.75);
	owner->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_BoostExpired);
	owner->GetWorld()->GetTimerManager().SetTimer(TimerHandle_BoostExpired, this, &USpeedBoost::ResetMovementSpeed, GetCooldown());
}

void USpeedBoost::ResetMovementSpeed(){
	if (owner){
		owner->SetSpeed(owner->originalData.movementSpeed);
	}
}

USpeedBoost* USpeedBoost::CreateAbility(){
	return NewObject<USpeedBoost>(USpeedBoost::StaticClass());
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "SpeedBoost.h"
#include "Ship.h"

void USpeedBoost::Activate(class AShip* abilityOwner){
	this->owner = abilityOwner;
	abilityOwner->SetSpeed(abilityOwner->GetMoveSpeed() * 1.75);
	abilityOwner->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_BoostExpired);
	abilityOwner->GetWorld()->GetTimerManager().SetTimer(TimerHandle_BoostExpired, this, &USpeedBoost::ResetMovementSpeed, GetCooldown());
}

void USpeedBoost::ResetMovementSpeed(){
	if (owner){
		owner->SetSpeed(owner->getOriginalData() .movementSpeed);
	}
}

USpeedBoost* USpeedBoost::CreateAbility(){
	return NewObject<USpeedBoost>(USpeedBoost::StaticClass());
}
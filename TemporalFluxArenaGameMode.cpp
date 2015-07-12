// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "TemporalFluxArena.h"
#include "TemporalFluxArenaGameMode.h"
#include "BaseHUD.h"
#include "Ship.h"
#include "ShipController.h"

ATemporalFluxArenaGameMode::ATemporalFluxArenaGameMode()
{
	static ConstructorHelpers::FClassFinder<AHUD> hud(TEXT("/Game/TwinStick/HUD.HUD_C"));
	// set default pawn class to our character class
	DefaultPawnClass = AShip::StaticClass();
	HUDClass = hud.Class;
	PlayerControllerClass = AShipController::StaticClass();
}


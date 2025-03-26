// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"

#define CUSTOM_DEPTH_RED 250

#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1

static void LogOnScreen(UObject *WorldContext, FString Msg, FColor Color = FColor::White, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] : " : "[SERVER] : ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}
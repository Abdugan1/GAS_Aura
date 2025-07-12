// Copyright Abdu Inc.


#include "Actor/MagicCircle.h"

#include "Components/DecalComponent.h"


AMagicCircle::AMagicCircle()
{
	PrimaryActorTick.bCanEverTick = true;

	MagicCircleDecalComponent = CreateDefaultSubobject<UDecalComponent>("Magic Circle Decal Component");
	MagicCircleDecalComponent->SetupAttachment(GetRootComponent());
}


void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	
}


void AMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"


UCLASS(Blueprintable)
class MEDJD_PROJETO1_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();
	
	/// Called upon interaction with owner actor, implemented in blueprints
	UFUNCTION(BlueprintImplementableEvent)
		void OnInteractionBP(class APlayerCharacter* Player);

	virtual void OnInteractionCPP(class APlayerCharacter* Player);
		
};

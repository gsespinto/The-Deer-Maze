// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"
#include "PlayerCharacter.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Tick function isn't needed in this component
	PrimaryComponentTick.bCanEverTick = false;
}

/// Called upon interaction with owner actor
void UInteractableComponent::OnInteractionCPP(APlayerCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Player interacted with '%s' object."), *GetOwner()->GetName());
	return;
}

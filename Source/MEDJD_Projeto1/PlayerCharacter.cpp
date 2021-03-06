// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "MyEnums.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "InteractableComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"

/// Constructor
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetMesh(), TEXT("Head"));
	SpringArm->bEditableWhenInherited = true;
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bEditableWhenInherited = true;

	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Trigger"));
	InteractionTrigger->SetupAttachment(SpringArm);
	InteractionTrigger->bEditableWhenInherited = true;

	OrbMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetupAttachment(RootComponent);
	OrbMesh->bEditableWhenInherited = true;
}

/// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Assign interaction trigger overlap functions
	InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionBeginOverlap);
	InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionEndOverlap);

	SetMovementStatus(EMovementStatus::WALK);
	SetOrbMaterial();
}

/// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForKeys();
	SetOrbMaterial();
}

/// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &APlayerCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &APlayerCharacter::Interact);

	// Custom input delegate to set movement status by action input
	DECLARE_DELEGATE_OneParam(FMovementStatusDelegate, EMovementStatus);
	PlayerInputComponent->BindAction<FMovementStatusDelegate>(TEXT("Run"), IE_Pressed, this, &APlayerCharacter::SetMovementStatus, EMovementStatus::RUN);
	PlayerInputComponent->BindAction<FMovementStatusDelegate>(TEXT("Run"), EInputEvent::IE_Released, this, &APlayerCharacter::SetMovementStatus, EMovementStatus::WALK).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction<FMovementStatusDelegate>(TEXT("Sneak"), IE_Pressed, this, &APlayerCharacter::SetMovementStatus, EMovementStatus::SNEAK);
	PlayerInputComponent->BindAction<FMovementStatusDelegate>(TEXT("Sneak"), EInputEvent::IE_Released, this, &APlayerCharacter::SetMovementStatus, EMovementStatus::WALK).bExecuteWhenPaused = true;
}

/// Handles if interactables enter range
void APlayerCharacter::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// If the overlapping actor doesn't contain an interactablecomponent
	// do nothing
	UInteractableComponent* Interactable = (UInteractableComponent*)OtherActor->FindComponentByClass(UInteractableComponent::StaticClass());
	if (!Interactable)
		return;

	// Add interactable component to the in range array
	InteractablesInRange.AddUnique(Interactable);
}

/// Handles if interactables leave range
void APlayerCharacter::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// If the overlapping actor doesn't contain an interactablecomponent
	// do nothing
	UInteractableComponent* Interactable = (UInteractableComponent*)OtherActor->FindComponentByClass(UInteractableComponent::StaticClass());
	if (!Interactable)
		return;

	// Remove interactable component to the in range array
	InteractablesInRange.Remove(Interactable);
}

/// Calls interact functions from the closest interactable within range
void APlayerCharacter::Interact()
{
	UInteractableComponent* ClosestInteractable = nullptr;

	for (int i = 0; i < InteractablesInRange.Num(); i++)
	{
		// Nullptr protection
		if (!InteractablesInRange[i] || !InteractablesInRange[i]->GetOwner())
		{
			InteractablesInRange.Remove(InteractablesInRange[i]);
			i--;
			continue;
		}

		// if there's no interactable selected
		// select i as closest
		if (!ClosestInteractable)
		{
			ClosestInteractable = InteractablesInRange[i];
			continue;
		}

		// if this interactable is closer to the player than the current
		// set this as current
		if (FVector::Distance(InteractablesInRange[i]->GetOwner()->GetActorLocation(), GetActorLocation()) < FVector::Distance(ClosestInteractable->GetOwner()->GetActorLocation(), GetActorLocation()))
		{
			ClosestInteractable = InteractablesInRange[i];
		}
	}

	// If no valid interactable was found
	// do nothing
	if (!ClosestInteractable)
	{
		if (CantInteractMontage)
			GetMesh()->GetAnimInstance()->Montage_Play(CantInteractMontage);
		// UE_LOG(LogTemp, Error, TEXT("Couldn't find a valid interactable within range."));
		return;
	}

	// Play interact animation montage
	if (InteractMontage)
		GetMesh()->GetAnimInstance()->Montage_Play(InteractMontage);

	// Execute interaction functions
	ClosestInteractable->OnInteractionBP(this);

	if(ClosestInteractable)
		ClosestInteractable->OnInteractionCPP(this);

	// If the interactable was destroyed
	// remove from the array
	if (!ClosestInteractable)
		InteractablesInRange.Remove(ClosestInteractable);
}

/// Checks for key actors within range and stores reference to the closest
void APlayerCharacter::CheckForKeys()
{
	// Nullptr protection
	if (!OrbMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s hasn't found a valid OrbMesh reference."), *GetName());
		return;
	}

	// Sphere cast to detected keys within range of player
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> SphereHits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), this->GetActorLocation(), this->GetActorLocation(), KeyDetectionRange, KeyObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, SphereHits, false);

	// If no key was found
	// Reset current key ref
	if (SphereHits.Num() <= 0)
	{
		CurrentKey = nullptr;
		return;
	}

	for (int i = 0; i < SphereHits.Num(); i++)
	{
		// Set current key ref if it hasn't been done yet
		if (!CurrentKey)
		{
			CurrentKey = SphereHits[i].GetActor();
			continue;
		}

		// If i key is closer to the player than the current
		// set i as current key ref
		if (FVector::Distance(SphereHits[i].GetActor()->GetActorLocation(), OrbMesh->GetComponentLocation()) < FVector::Distance(CurrentKey->GetActorLocation(), OrbMesh->GetComponentLocation()))
			CurrentKey = SphereHits[i].GetActor();
	}
}

/// Sets Orb Material according to distance to nearest key
void APlayerCharacter::SetOrbMaterial()
{
	// Nullptr protection
	if(!OrbMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s hasn't found a valid OrbMesh reference."), *GetName());
		return;
	}

	// If there's no key nearby
	// Set orb material to deactivated
	if (!CurrentKey)
	{
		// Nullptr protection
		if (!OrbDeactivatedMat)
		{
			UE_LOG(LogTemp, Error, TEXT("No 'OrbDeactivatedMat' reference was found in %s"), *GetName());
			return;
		}

		OrbMesh->SetMaterial(0, OrbDeactivatedMat);
		return;
	}

	// Set orb material to first material that is within distance range
	const float DistanceToKey = FVector::Distance(CurrentKey->GetActorLocation(), OrbMesh->GetComponentLocation());
	for (int i = 0; i < OrbMaterials.Num(); i++)
	{
		if (DistanceToKey > OrbMaterials[i].MaxDistanceFactorForUsage * KeyDetectionRange)
			continue;

		if (OrbMesh->GetMaterial(0) != OrbMaterials[i].Material)
			OrbMesh->SetMaterial(0, OrbMaterials[i].Material);
		break;
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void APlayerCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

/// Sets current movement status, changing the character's max speed value
void APlayerCharacter::SetMovementStatus(EMovementStatus NewStatus)
{
	switch (NewStatus)
	{
		case EMovementStatus::WALK:
			GetCharacterMovement()->bWantsToCrouch = false;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;
		case EMovementStatus::RUN:
			GetCharacterMovement()->bWantsToCrouch = false;
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
		case EMovementStatus::SNEAK:
			GetCharacterMovement()->bWantsToCrouch = true;
			break;

		default:
			GetCharacterMovement()->bWantsToCrouch = false;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;
	}
}

/// Plays random step sfx from they array on the player's location, with given volume modifier
void APlayerCharacter::PlayStepSFX(float VolumeModifier)
{
	// Nullptr protection
	if (StepsSFX.Num() <= 0)
		return;
		
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), StepsSFX[FMath::RandRange(0, StepsSFX.Num() - 1)], GetActorLocation(), GetActorRotation(), VolumeModifier);
}

void APlayerCharacter::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
}



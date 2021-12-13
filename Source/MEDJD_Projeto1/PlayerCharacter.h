// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyEnums.h"
#include "MyStructs.h"
#include "Engine/EngineTypes.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MEDJD_PROJETO1_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// FX
	UFUNCTION(BlueprintCallable)
		void PlayStepSFX(float VolumeModifier = 1.0f);

	// INTERACTION
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FString> KeyNames;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CAMERA
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, Category = Interaction)
		class UBoxComponent* InteractionTrigger;

	// INTERACTION
	UPROPERTY(BlueprintReadOnly)
		TArray<class UInteractableComponent*> InteractablesInRange;
	UFUNCTION()
		virtual void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Orb)
		class USkeletalMeshComponent* OrbMesh;

private:
	// CAMERA
	UPROPERTY(EditAnywhere)
		float LookUpRate = 60.0f;
	void LookUpAtRate(float Value);
	UPROPERTY(EditAnywhere)
		float TurnRate = 60.0f;
	void TurnAtRate(float Value);
	
	// MOVEMENT
	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(EditAnywhere, Category = Movement)
		float WalkSpeed = 350.0f;
	UPROPERTY(EditAnywhere, Category = Movement)
		float RunSpeed = 600.0f;

	void SetMovementStatus(EMovementStatus NewStatus);

	void Interact();
	
	void CheckForKeys();
	AActor* CurrentKey;
	UPROPERTY(EditAnywhere, Category = Interaction)
		float KeyDetectionRange = 250.0f;
	UPROPERTY(EditDefaultsOnly, Category = Orb)
		TArray<TEnumAsByte<EObjectTypeQuery>> KeyObjectTypes;

	UPROPERTY(EditDefaultsOnly, Category = Interaction)
		class UAnimMontage* InteractMontage;
	UPROPERTY(EditDefaultsOnly, Category = Interaction)
		class UAnimMontage* CantInteractMontage;

	// FX 
	UPROPERTY(EditAnywhere)
		TArray<class USoundWave*> StepsSFX;

	// ORB
	UPROPERTY(EditDefaultsOnly, Category = Orb)
		class UMaterialInterface* OrbDeactivatedMat;
	UPROPERTY(EditDefaultsOnly, Category = Orb)
		TArray<FOrbMaterial> OrbMaterials;
	void SetOrbMaterial();

};

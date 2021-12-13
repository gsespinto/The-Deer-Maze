// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyStructs.generated.h"

USTRUCT(Blueprintable)
struct FOrbMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Orb)
		class UMaterialInterface* Material;
	UPROPERTY(EditDefaultsOnly, Category = Orb, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxDistanceFactorForUsage = 0.0f;
};
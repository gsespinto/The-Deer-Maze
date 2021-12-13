// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum EMovementStatus
{
    WALK     UMETA(DisplayName = "Walk"),
    RUN      UMETA(DisplayName = "Run"),
    SNEAK   UMETA(DisplayName = "Crouch"),
};

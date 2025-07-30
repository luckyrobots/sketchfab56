// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SketchfabSettings.generated.h"

UCLASS(Config = Sketchfab, defaultconfig)
class SKETCHFABUNREAL_API USketchfabSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Sketchfab")
	FString ClientId;

	UPROPERTY(Config, EditAnywhere, Category = "Sketchfab")
	FString ClientSecret;
}; 
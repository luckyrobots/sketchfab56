// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "sketchfab56PlayerController.h"
#include "MyPlayerController.generated.h"

class USketchfabSearchWidget;

UCLASS()
class SKETCHFAB56_API AMyPlayerController : public Asketchfab56PlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USketchfabSearchWidget> SketchfabSearchWidgetClass;

	virtual void BeginPlay() override;
}; 
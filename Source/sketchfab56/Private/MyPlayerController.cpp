// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "SketchfabSearchWidget.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (SketchfabSearchWidgetClass)
	{
		USketchfabSearchWidget* SearchWidget = CreateWidget<USketchfabSearchWidget>(this, SketchfabSearchWidgetClass);
		if (SearchWidget)
		{
			SearchWidget->AddToViewport();
		}
	}
} 
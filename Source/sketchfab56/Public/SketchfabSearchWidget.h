// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SketchfabData.h"
#include "SketchfabSearchWidget.generated.h"

class UEditableTextBox;
class UButton;
class UListView;

UCLASS()
class SKETCHFAB56_API USketchfabSearchWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SearchBox;

	UPROPERTY(meta = (BindWidget))
	UButton* SearchButton;

	UPROPERTY(meta = (BindWidget))
	UListView* ResultsList;

	UFUNCTION()
	void OnSearchButtonClicked();

	UFUNCTION()
	void OnModelSearchCompleted(const FSketchfabSearchResponse& Response);
	
	UFUNCTION()
	void OnModelImported(const FSketchfabModelImportResponse& Response);

	// Temporary hard-coded auth callback
	UFUNCTION()
	void HandleAuthResponse(const FSketchfabAccessTokenResponse& Response);
}; 
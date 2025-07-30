// Copyright Epic Games, Inc. All Rights Reserved.

#include "SketchfabSearchWidget.h"
#include "SketchfabSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ListView.h"

void USketchfabSearchWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SearchButton)
	{
		SearchButton->OnClicked.AddDynamic(this, &USketchfabSearchWidget::OnSearchButtonClicked);
	}

	USketchfabSubsystem* Sketchfab = GetGameInstance()->GetSubsystem<USketchfabSubsystem>();
	if (Sketchfab)
	{
		// --- SET TOKEN DIRECTLY ---
		Sketchfab->SetAccessToken(TEXT("YOURTOKEN")); 

		Sketchfab->OnModelSearchCompleted.AddDynamic(this, &USketchfabSearchWidget::OnModelSearchCompleted);
		Sketchfab->OnModelImported.AddDynamic(this, &USketchfabSearchWidget::OnModelImported);
	}
}

void USketchfabSearchWidget::OnSearchButtonClicked()
{
	if (SearchBox && !SearchBox->GetText().IsEmpty())
	{
		USketchfabSubsystem* Sketchfab = GetGameInstance()->GetSubsystem<USketchfabSubsystem>();
		if (Sketchfab)
		{
			FSketchfabSearchRequest Request;
			Request.Keywords = SearchBox->GetText().ToString();
			Sketchfab->SearchForModels(Request);
		}
	}
}

void USketchfabSearchWidget::OnModelSearchCompleted(const FSketchfabSearchResponse& Response)
{
	if (!Response.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Search failed: %s"), *Response.Error);
		return;
	}

	if (Response.Results.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Found %d models, importing first..."), Response.Results.Num());
		USketchfabSubsystem* Sketchfab = GetGameInstance()->GetSubsystem<USketchfabSubsystem>();
		if (Sketchfab)
		{
			FSketchfabImportOptions Options;
			Sketchfab->ImportModel(Response.Results[0].Uid, Options);
		}
	}
}

void USketchfabSearchWidget::OnModelImported(const FSketchfabModelImportResponse& Response)
{
	if (Response.bSuccess && Response.SpawnedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Model spawned successfully!"));
	}
	else if (!Response.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Import failed: %s"), *Response.Error);
	}
}

void USketchfabSearchWidget::HandleAuthResponse(const FSketchfabAccessTokenResponse& Response)
{
	if (Response.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Authentication succeeded."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Authentication failed: %s"), *Response.Error);
	}
}

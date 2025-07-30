// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SketchfabData.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "SketchfabSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessTokenReceived, const FSketchfabAccessTokenResponse&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModelSearchCompleted, const FSketchfabSearchResponse&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModelImported, const FSketchfabModelImportResponse&, Response);


UCLASS()
class SKETCHFABUNREAL_API USketchfabSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Sketchfab")
	void RequestAccessToken(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Sketchfab")
	void SearchForModels(const FSketchfabSearchRequest& SearchRequest);

	UFUNCTION(BlueprintCallable, Category = "Sketchfab")
	void ImportModel(const FString& ModelUid, const FSketchfabImportOptions& ImportOptions);

	UFUNCTION(BlueprintCallable, Category = "Sketchfab")
	void SetAccessToken(const FString& NewToken);

public:
	UPROPERTY(BlueprintAssignable, Category = "Sketchfab")
	FOnAccessTokenReceived OnAccessTokenReceived;

	UPROPERTY(BlueprintAssignable, Category = "Sketchfab")
	FOnModelSearchCompleted OnModelSearchCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sketchfab")
	FOnModelImported OnModelImported;

private:
	FString AccessToken;

	void OnAccessTokenRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnSearchRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnModelDownloadUrlRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString ModelUid, FSketchfabImportOptions ImportOptions);
	void OnModelDataDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString ModelUid, FSketchfabImportOptions ImportOptions);
}; 
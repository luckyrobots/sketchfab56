// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SketchfabData.generated.h"

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString Uid;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString Name;
};

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabSearchRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	FString Keywords;

	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	bool bDownloadable = true;
	
	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	int32 MinFaceCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	int32 MaxFaceCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	TArray<FString> Tags;

	FSketchfabSearchRequest() = default;
};

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabSearchResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString Error;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	TArray<FSketchfabModel> Results;
};

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabImportOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Sketchfab")
	bool bUseCache = true;

	FSketchfabImportOptions() = default;
};

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabModelImportResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString Error;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FSketchfabModel ModelInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	AActor* SpawnedActor = nullptr;
};

USTRUCT(BlueprintType)
struct SKETCHFABUNREAL_API FSketchfabAccessTokenResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString Error;
	
	UPROPERTY(BlueprintReadOnly, Category = "Sketchfab")
	FString AccessToken;
};

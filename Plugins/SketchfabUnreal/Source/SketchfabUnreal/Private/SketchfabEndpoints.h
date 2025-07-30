// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace SketchfabEndpoints
{
	const FString AccessToken = TEXT("https://sketchfab.com/oauth2/token/");
	const FString ModelSearch = TEXT("https://api.sketchfab.com/v3/search?type=models");
	const FString ModelDownload = TEXT("https://api.sketchfab.com/v3/models/{0}/download");
} 
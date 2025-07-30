// Copyright Epic Games, Inc. All Rights Reserved.

#include "SketchfabSubsystem.h"
#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "SketchfabEndpoints.h"
#include "SketchfabSettings.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/Base64.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "glTFRuntimeFunctionLibrary.h"

void USketchfabSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USketchfabSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USketchfabSubsystem::RequestAccessToken(const FString& Email, const FString& Password)
{
	const USketchfabSettings* Settings = GetDefault<USketchfabSettings>();
	if (!Settings || Settings->ClientId.IsEmpty() || Settings->ClientSecret.IsEmpty())
	{
		FSketchfabAccessTokenResponse Response;
		Response.bSuccess = false;
		Response.Error = TEXT("Client ID or Client Secret is not configured in Project Settings.");
		OnAccessTokenReceived.Broadcast(Response);
		return;
	}

	// Build form body (grant_type + credentials)
FString RequestBody = FString::Printf(TEXT("grant_type=password&username=%s&password=%s"),
    *FGenericPlatformHttp::UrlEncode(Email),
    *FGenericPlatformHttp::UrlEncode(Password));

// Build Basic authorization header (base64(client_id:client_secret))
FString RawCreds = Settings->ClientId + TEXT(":") + Settings->ClientSecret;
FString EncodedCreds = FBase64::Encode(RawCreds);

FHttpModule& HttpModule = FHttpModule::Get();
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
Request->SetURL(SketchfabEndpoints::AccessToken);
Request->SetVerb("POST");
Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
Request->SetHeader("Authorization", FString::Printf(TEXT("Basic %s"), *EncodedCreds));
Request->SetHeader("Accept", "application/json");
Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &USketchfabSubsystem::OnAccessTokenRequestCompleted);
	Request->ProcessRequest();
}

void USketchfabSubsystem::SearchForModels(const FSketchfabSearchRequest& SearchRequest)
{
	if (AccessToken.IsEmpty())
	{
		FSketchfabSearchResponse Response;
		Response.bSuccess = false;
		Response.Error = TEXT("Not authenticated. Please request an access token first.");
		OnModelSearchCompleted.Broadcast(Response);
		return;
	}

	FString Url = SketchfabEndpoints::ModelSearch;
	Url += "&q=" + FGenericPlatformHttp::UrlEncode(SearchRequest.Keywords);

	if (SearchRequest.bDownloadable)
	{
		Url += "&downloadable=true";
	}
	if (SearchRequest.MinFaceCount > 0)
	{
		Url += "&min_face_count=" + FString::FromInt(SearchRequest.MinFaceCount);
	}
	if (SearchRequest.MaxFaceCount > 0)
	{
		Url += "&max_face_count=" + FString::FromInt(SearchRequest.MaxFaceCount);
	}
	if (SearchRequest.Tags.Num() > 0)
	{
		Url += "&tags=";
		for (int32 i = 0; i < SearchRequest.Tags.Num(); ++i)
		{
			Url += FGenericPlatformHttp::UrlEncode(SearchRequest.Tags[i]);
			if (i < SearchRequest.Tags.Num() - 1)
			{
				Url += "+";
			}
		}
	}

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb("GET");
	Request->SetHeader("Authorization", "Bearer " + AccessToken);
	Request->OnProcessRequestComplete().BindUObject(this, &USketchfabSubsystem::OnSearchRequestCompleted);
	Request->ProcessRequest();
}

void USketchfabSubsystem::ImportModel(const FString& ModelUid, const FSketchfabImportOptions& ImportOptions)
{
	if (AccessToken.IsEmpty())
	{
		FSketchfabModelImportResponse Response;
		Response.bSuccess = false;
		Response.Error = TEXT("Not authenticated. Please request an access token first.");
		OnModelImported.Broadcast(Response);
		return;
	}

	FString Url = FString::Format(*SketchfabEndpoints::ModelDownload, { ModelUid });

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb("GET");
	Request->SetHeader("Authorization", "Bearer " + AccessToken);
	Request->OnProcessRequestComplete().BindUObject(this, &USketchfabSubsystem::OnModelDownloadUrlRequestCompleted, ModelUid, ImportOptions);
	Request->ProcessRequest();
}

void USketchfabSubsystem::SetAccessToken(const FString& NewToken)
{
	AccessToken = NewToken;
}

void USketchfabSubsystem::OnAccessTokenRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FSketchfabAccessTokenResponse ResponseData;
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject->HasField("access_token"))
			{
				AccessToken = JsonObject->GetStringField("access_token");
				ResponseData.bSuccess = true;
				ResponseData.AccessToken = AccessToken;
			}
			else
			{
				ResponseData.Error = JsonObject->HasField("error_description") ? JsonObject->GetStringField("error_description") : TEXT("Unknown error getting access token.");
			}
		}
		else
		{
			ResponseData.Error = TEXT("Failed to parse JSON response.");
		}
	}
	else
	{
		ResponseData.Error = FString::Printf(TEXT("Request failed. Code: %d, Message: %s"), Response.IsValid() ? Response->GetResponseCode() : 0, *Response->GetContentAsString());
	}

	OnAccessTokenReceived.Broadcast(ResponseData);
}

void USketchfabSubsystem::OnSearchRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FSketchfabSearchResponse ResponseData;
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* ResultsJson;
			if (JsonObject->TryGetArrayField("results", ResultsJson))
			{
				for (const TSharedPtr<FJsonValue>& Val : *ResultsJson)
				{
					TSharedPtr<FJsonObject> ModelObj = Val->AsObject();
					if (ModelObj.IsValid())
					{
						FSketchfabModel Model;
						if (FJsonObjectConverter::JsonObjectToUStruct(ModelObj.ToSharedRef(), FSketchfabModel::StaticStruct(), &Model))
						{
							ResponseData.Results.Add(Model);
						}
					}
				}
				ResponseData.bSuccess = true;
			}
			else
			{
				ResponseData.Error = TEXT("Missing 'results' field in search response.");
			}
		}
		else
		{
			ResponseData.Error = TEXT("Failed to parse JSON response.");
		}
	}
	else
	{
		ResponseData.Error = FString::Printf(TEXT("Search request failed. Code: %d, Message: %s"), Response.IsValid() ? Response->GetResponseCode() : 0, *Response->GetContentAsString());
	}

	OnModelSearchCompleted.Broadcast(ResponseData);
}

void USketchfabSubsystem::OnModelDownloadUrlRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString ModelUid, FSketchfabImportOptions ImportOptions)
{
	FSketchfabModelImportResponse ResponseData;
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* GltfObject;
			if (JsonObject->TryGetObjectField("gltf", GltfObject))
			{
				FString DownloadUrl = (*GltfObject)->GetStringField("url");
				
				FHttpModule& HttpModule = FHttpModule::Get();
				TSharedRef<IHttpRequest, ESPMode::ThreadSafe> DownloadRequest = HttpModule.CreateRequest();
				DownloadRequest->SetURL(DownloadUrl);
				DownloadRequest->SetVerb("GET");
				DownloadRequest->OnProcessRequestComplete().BindUObject(this, &USketchfabSubsystem::OnModelDataDownloaded, ModelUid, ImportOptions);
				DownloadRequest->ProcessRequest();

				// Early exit, the actual broadcast will happen in OnModelDataDownloaded
				return;
			}
			else
			{
				ResponseData.Error = TEXT("Could not find gltf download URL in response.");
			}
		}
		else
		{
			ResponseData.Error = TEXT("Failed to parse model download URL JSON response.");
		}
	}
	else
	{
		ResponseData.Error = FString::Printf(TEXT("Model download URL request failed. Code: %d, Message: %s"), Response.IsValid() ? Response->GetResponseCode() : 0, *Response->GetContentAsString());
	}

	OnModelImported.Broadcast(ResponseData);
}

void USketchfabSubsystem::OnModelDataDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString ModelUid, FSketchfabImportOptions ImportOptions)
{
	FSketchfabModelImportResponse ResponseData;
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		FString TmpPath = FPaths::ProjectIntermediateDir() / TEXT("SketchfabDownloads");
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*TmpPath))
		{
			PlatformFile.CreateDirectory(*TmpPath);
		}

		FString Filename = TmpPath / (ModelUid + ".gltf");
		if (FFileHelper::SaveArrayToFile(Response->GetContent(), *Filename))
		{
			// Load the glTF asset (returns UglTFRuntimeAsset). Spawning an actor
			// is left for future implementation; we only verify the asset loads.
			FglTFRuntimeConfig RuntimeConfig;
			UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(Filename, false, RuntimeConfig);
			if (Asset)
			{
				ResponseData.bSuccess = true;
			}
			else
			{
				ResponseData.Error = TEXT("Failed to load glTF asset.");
			}
		}
		else
		{
			ResponseData.Error = TEXT("Failed to save downloaded model data to file.");
		}
	}
	else
	{
		ResponseData.Error = FString::Printf(TEXT("Model data download failed. Code: %d"), Response.IsValid() ? Response->GetResponseCode() : 0);
	}

	OnModelImported.Broadcast(ResponseData);
} 
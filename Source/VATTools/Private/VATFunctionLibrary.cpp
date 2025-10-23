// Fill out your copyright notice in the Description page of Project Settings.

#include "VATFunctionLibrary.h"
#include "Editor/UnrealEdEngine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MaterialEditingLibrary.h"


#if WITH_EDITOR
// Mark asset dirty so it can be overwritten & saved
void UVATFunctionLibrary::MarkAssetDirty(UObject* a_Asset, bool& a_bDirtySuccess, FString& a_Warning)
{
	// Setup defaults
	a_Warning = "";
	a_bDirtySuccess = true;

	// Check if input is valid
	if (!IsValid(a_Asset))
	{
		a_Warning = "Provided asset is invalid";
		a_bDirtySuccess = false;
		return;
	}

	// Mark asset dirty
	a_bDirtySuccess = a_Asset->MarkPackageDirty();
	if (a_bDirtySuccess)
	{
		a_Asset->PostEditChange();
	}
	else
	{
		a_Warning = "Could not mark package dirty";
	}
}

// Process VAT mesh
void UVATFunctionLibrary::SetupVATMesh(
	UStaticMesh* a_VATMesh, 
	bool& a_bMeshSetupSuccess,
	FString& a_Warning,
	const bool& a_bUseFullPrecision,
	const FVector& a_MeshExtendsMax,
	const FVector& a_MeshExtendsMin
)
{
	// Set defaults
	a_bMeshSetupSuccess = true;
	a_Warning = "";

	// Validate input
	if (!IsValid(a_VATMesh))
	{
		a_bMeshSetupSuccess = false;
		a_Warning = "Provided mesh is not valid";
		return;
	}

	// Disable nanite
	a_VATMesh->NaniteSettings.bEnabled = false;

	// Update the mesh extents
	a_VATMesh->SetPositiveBoundsExtension(a_MeshExtendsMax);
	a_VATMesh->SetNegativeBoundsExtension(a_MeshExtendsMin);

	// Enable full precision UVs (if necessary)
	FStaticMeshSourceModel& SourceModel = a_VATMesh->GetSourceModel(0);
	SourceModel.BuildSettings.bUseFullPrecisionUVs = a_bUseFullPrecision;

	// Disable auto compute UVs and tangents
	SourceModel.BuildSettings.bRecomputeNormals = false;
	SourceModel.BuildSettings.bRecomputeTangents = false;
	
	// Update mesh
	a_VATMesh->Build();
	a_bMeshSetupSuccess = a_VATMesh->MarkPackageDirty();
	if (a_bMeshSetupSuccess) 
	{
		a_VATMesh->PostEditChange();
	}
	else
	{
		a_Warning = "Could not mark package dirty";
	}
}

// Reads a JSON file and converts it into a string
TSharedPtr<FJsonObject> UVATFunctionLibrary::GetJSONData(const FFilePath& a_JSONFile, bool& a_bOutSuccess, FString& a_Warning)
{
	// Set defaults 
	FString JSONString = "";
	a_bOutSuccess = true;
	a_Warning = "";

	// Check if the file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*a_JSONFile.FilePath))
	{
		a_bOutSuccess = false;
		a_Warning = "File does not exist";
		return nullptr;
	}

	// Read the file
	if (!FFileHelper::LoadFileToString(JSONString, *a_JSONFile.FilePath))
	{
		a_bOutSuccess = false;
		a_Warning = "Cannot read the file";
		return nullptr;
	}

	// Create the JSON object
	TSharedPtr<FJsonObject> JSONObject;

	// Convert JSON string to JSON object
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JSONString), JSONObject))
	{
		a_bOutSuccess = false;
		a_Warning = "Could not convert JSON file to JSON object";
		return nullptr;
	}

	// Return
	return JSONObject;
}

// Read a JSON float
float UVATFunctionLibrary::GetJSONFloat(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning)
{
	// Get JSON object
	TSharedPtr<FJsonObject> JSONObject = GetJSONData(a_JSONFile, a_bOutSuccess, a_Warning);
	if (!a_bOutSuccess)
	{
		return 0.0f;
	}

	// Read the JSON object
	float JSONFloat = 0.0f;
	if (!JSONObject->TryGetNumberField(a_FieldName, JSONFloat))
	{
		a_bOutSuccess = false;
		a_Warning = "Could not read JSON float with the given field name. Check if name is correcet, and if the field contains a number.";
		return 0.0f;
	}

	return JSONFloat;
}

// Read a JSON vector
FVector UVATFunctionLibrary::GetJSONVector(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning)
{
	// Get JSON object
	TSharedPtr<FJsonObject> JSONObject = GetJSONData(a_JSONFile, a_bOutSuccess, a_Warning);
	if (!a_bOutSuccess)
	{
		return FVector(0.0f, 0.0f, 0.0f);
	}

	// Read the JSON object
	FVector JSONVector = FVector(0.0f, 0.0f, 0.0f);

	const TArray<TSharedPtr<FJsonValue>>* JSONValue;
	if (!JSONObject->TryGetArrayField(a_FieldName, JSONValue))
	{
		a_Warning = "Could not get the field with the given field name";
		a_bOutSuccess = false;
		return FVector(0.0f, 0.0f, 0.0f);
	}

	// Create the array
	TArray<TSharedPtr<FJsonValue>> SampleArray = JSONObject->GetArrayField(a_FieldName);
	for (int i = 0; i < 3; i++)
	{
		if (!SampleArray.IsValidIndex(i))
		{
			JSONVector[i] = 0.0f;
		}
		else
		{
			JSONVector[i] = SampleArray[i]->AsNumber();
		}
	}

	return JSONVector;
}

FString UVATFunctionLibrary::GetJSONString(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning)
{
	// Get JSON object
	TSharedPtr<FJsonObject> JSONObject = GetJSONData(a_JSONFile, a_bOutSuccess, a_Warning);
	if (!a_bOutSuccess)
	{
		return "";
	}

	// Read the JSON object
	FString JSONString = "";
	if (!JSONObject->TryGetStringField(a_FieldName, JSONString))
	{
		a_bOutSuccess = false;
		a_Warning = "Could not read JSON float with the given field name. Check if name is correcet, and if the field contains a number.";
		return "";
	}
	return JSONString;
}

void UVATFunctionLibrary::SetMaterialInstanceScalar(UMaterialInterface* a_Material, const float& a_scalar, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning)
{
	a_bOutSuccess = true;
	a_Warning = "";

	// Check if the provided material interface is valid
	if (a_Material == nullptr)
	{
		a_bOutSuccess = false;
		a_Warning = "Invalid material instance provided";
		return;
	}

	// Check if we have a material or material instance
	UMaterial* Material = Cast<UMaterial>(a_Material);
	UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(a_Material);

	if (Material != nullptr)
	{
		// Set parameter
		Material->SetScalarParameterValueEditorOnly(FName(a_ParameterName), a_scalar);
		UMaterialEditingLibrary::RecompileMaterial(Material);
	}
	else if (MaterialInstance != nullptr)
	{	
		// Set parameter
		MaterialInstance->SetScalarParameterValueEditorOnly(FName(a_ParameterName), a_scalar);
		UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);
	}
	else
	{
		a_bOutSuccess = false;
		a_Warning = "The given material interface is not of material or a material instance class";
		return;
	}

	return;
}

void UVATFunctionLibrary::SetMaterialInstanceVector(UMaterialInterface* a_Material, const FVector& a_Vector, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning)
{
	a_bOutSuccess = true;
	a_Warning = "";

	// Check if the provided material interface is valid
	if (a_Material == nullptr)
	{
		a_bOutSuccess = false;
		a_Warning = "Invalid material instance provided";
		return;
	}

	// Check if we have a material or material instance
	UMaterial* Material = Cast<UMaterial>(a_Material);
	UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(a_Material);
	FLinearColor ParameterValue(a_Vector.X, a_Vector.Y, a_Vector.Z);
	if (Material != nullptr)
	{
		// Set parameter
		Material->SetVectorParameterValueEditorOnly(FName(a_ParameterName), ParameterValue);
		UMaterialEditingLibrary::RecompileMaterial(Material);
	}
	else if (MaterialInstance != nullptr)
	{
		// Set parameter
		MaterialInstance->SetVectorParameterValueEditorOnly(FName(a_ParameterName), ParameterValue);
		UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);
	}
	else
	{
		a_bOutSuccess = false;
		a_Warning = "The given material interface is not of material or a material instance class";
		return;
	}
	return;
}

void UVATFunctionLibrary::SetMaterialInstanceStaticBool(UMaterialInterface* a_Material, const bool& a_Bool, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning)
{
	a_bOutSuccess = true;
	a_Warning = "";

	// Check if the provided material interface is valid
	if (a_Material == nullptr)
	{
		a_bOutSuccess = false;
		a_Warning = "Invalid material instance provided";
		return;
	}

	// Check if we have a material or material instance
	UMaterial* Material = Cast<UMaterial>(a_Material);
	UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(a_Material);
	if (Material != nullptr)
	{
		// Set parameter
		Material->SetStaticSwitchParameterValueEditorOnly(FName(a_ParameterName), a_Bool, FGuid());
		UMaterialEditingLibrary::RecompileMaterial(Material);
	}
	else if (MaterialInstance != nullptr)
	{
		// Set parameter
		MaterialInstance->SetStaticSwitchParameterValueEditorOnly(FName(a_ParameterName), a_Bool);
		UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);
	}
	else
	{
		a_bOutSuccess = false;
		a_Warning = "The given material interface is not of material or a material instance class";
		return;
	}
	return;
}

void UVATFunctionLibrary::SetMaterialInstanceTexture(UMaterialInterface* a_Material, UTexture2D* a_Texture, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning)
{
	a_bOutSuccess = true;
	a_Warning = "";

	// Check if the provided material interface is valid
	if (a_Material == nullptr)
	{
		a_bOutSuccess = false;
		a_Warning = "Invalid material instance provided";
		return;
	}

	// Check if we have a material or material instance
	UMaterial* Material = Cast<UMaterial>(a_Material);
	UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(a_Material);
	if (Material != nullptr)
	{
		// Set parameter
		Material->SetTextureParameterValueEditorOnly(FName(a_ParameterName), a_Texture);
		UMaterialEditingLibrary::RecompileMaterial(Material);
	}
	else if (MaterialInstance != nullptr)
	{
		// Set parameter
		MaterialInstance->SetTextureParameterValueEditorOnly(FName(a_ParameterName), a_Texture);
		UMaterialEditingLibrary::UpdateMaterialInstance(MaterialInstance);
	}
	else
	{
		a_bOutSuccess = false;
		a_Warning = "The given material interface is not of material or a material instance class";
		return;
	}
	return;
}


#endif
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Json.h"
#include "Engine/EngineTypes.h"
#include "VATFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VATTOOLS_API UVATFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Mark asset dirty"))
	static void MarkAssetDirty(UObject* a_Asset, bool& a_bDirtySuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName =  "Setup VAT mesh"))
	static void SetupVATMesh(
		UStaticMesh* a_VATMesh, 
		bool& a_bMeshSetupSuccess,
		FString& a_Warning,
		const bool& a_bUseFullPrecision = false, 
		const FVector& a_MeshExtendsMax = FVector(0.0f, 0.0f, 0.0f),
		const FVector& a_MeshExtendsMin = FVector(0.0f, 0.0f, 0.0f)
	);

	static TSharedPtr<FJsonObject> GetJSONData(const FFilePath& a_JSONFile, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Get JSON float"))
	static float GetJSONFloat(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Get JSON vector"))
	static FVector GetJSONVector(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Get JSON string"))
	static FString GetJSONString(const FFilePath& a_JSONFile, const FString& a_FieldName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Set scalar parameter"))
	static void SetMaterialInstanceScalar(UMaterialInterface* a_Material, const float& a_Scalar, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Set vector parameter"))
	static void SetMaterialInstanceVector(UMaterialInterface* a_Material, const FVector& a_Vector, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Set bool parameter"))
	static void SetMaterialInstanceStaticBool(UMaterialInterface* a_Material, const bool& a_Bool, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning);

	UFUNCTION(BlueprintCallable, Category = "VATTools", meta = (DisplayName = "Set texture parameter"))
	static void SetMaterialInstanceTexture(UMaterialInterface* a_Material, UTexture2D* a_Texture, const FString& a_ParameterName, bool& a_bOutSuccess, FString& a_Warning);

	#endif
};

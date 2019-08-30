// Copyright Nicholas Ferrar 2019

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "PSData.generated.h"


/**
 * 
 */
UCLASS()
class NFPOPULATIONSYSTEM_API UPSData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	////////////////////
	/////SETTINGS///////
	////////////////////

public:

	///Actual Setters and getter
	//Setters
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetFloatByName(UObject* Target, FName VarName, float NewValue, float &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetIntByName(UObject* Target, FName VarName, int NewValue, int &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetInt64ByName(UObject* Target, FName VarName, int64 NewValue, int64 &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetBoolByName(UObject* Target, FName VarName, bool NewValue, bool &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetNameByName(UObject* Target, FName VarName, FName NewValue, FName &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetObjectByName(UObject* Target, FName VarName, UObject* NewValue, UObject* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetClassByName(UObject* Target, FName VarName, class UClass* NewValue, class UClass* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetByteByName(UObject* Target, FName VarName, uint8 NewValue, uint8 &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetStringByName(UObject* Target, FName VarName, FString NewValue, FString &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetTextByName(UObject* Target, FName VarName, FText NewValue, FText &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetStructByName(UObject* Target, FName VarName, UScriptStruct* NewValue, UScriptStruct* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool SetEnumByName(UObject* Target, FName VarName, uint8 NewValue, uint8 &OutValue);

	//Getters
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetFloatByName(UObject* Target, FName VarName, float &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetIntByName(UObject* Target, FName VarName, int &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetInt64ByName(UObject* Target, FName VarName, int64 &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetBoolByName(UObject* Target, FName VarName, bool &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetNameByName(UObject* Target, FName VarName, FName &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetObjectByName(UObject* Target, FName VarName, UObject* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetClassByName(UObject* Target, FName VarName, class UClass* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetByteByName(UObject* Target, FName VarName, uint8 &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetStringByName(UObject* Target, FName VarName, FString &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetTextByName(UObject* Target, FName VarName, FText &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetStructByName(UObject* Target, FName VarName, UScriptStruct* &OutValue);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
		static bool GetEnumByName(UObject* Target, FName VarName, uint8 &OutValue);

};

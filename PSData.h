// Copyright Nicholas Ferrar 2019

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Engine/DataTable.h"
#include "RandomStream.h"

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

	UFUNCTION(BlueprintCallable, category = "nfPopulationSystem|Settings")
		static bool DoesUseMiddleNames();

	UFUNCTION(BlueprintPure, category = "nfPopulationSystem|Settings")
		static UDataTable* GetFirstNamesData();

	UFUNCTION(BlueprintPure, category = "nfPopulationSystem|Settings")
		static UDataTable* GetMiddleNamesData();

	UFUNCTION(BlueprintPure, category = "nfPopulationSystem|Settings")
		static UDataTable* GetLastNamesData();

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

	/*
	UFUNCTION(BlueprintCallable, Category = "Population System|Helpers", CustomThunk, meta = (CustomStructureParam = "OutProp"))//, WorldContext = "Context"))//, DeterminesOutputType = "OutProp"))
		static class UProperty* GetVariableByName(UObject* Target, UPARAM(ref) const FString VariableName, UPARAM(ref) UProperty*& OutProp);

	DECLARE_FUNCTION(execGetVariableByName)
	{
		//Get properties and pointers from stack

		//Gets UObject* to Target
		Stack.StepCompiledIn<UObjectProperty>(NULL);
		UObjectProperty* TargetProp = ExactCast<UObjectProperty>(Stack.MostRecentProperty);
		UObject* Target = TargetProp->GetObjectPropertyValue(Stack.MostRecentPropertyAddress);

		//Get FName to VariableName
		Stack.StepCompiledIn<UStrProperty>(NULL);
		UStrProperty* VarNameProp = ExactCast<UStrProperty>(Stack.MostRecentProperty);
		FString StrValue = VarNameProp->GetPropertyValue(Stack.MostRecentPropertyAddress);
		FName VariableName = *StrValue;

		//Get UProperty to OutPropProp
		//Stack.StepCompiledIn<UBoolProperty>(NULL);
		Stack.StepCompiledIn<UProperty>(NULL);
		UProperty* OutPropProp = Stack.MostRecentProperty;
		//UBoolProperty* OutBoolProp = ExactCast<UBoolProperty>(OutPropProp);
		//UFloatProperty* OutFloatProp = ExactCast<UFloatProperty>(OutPropProp);

		Stack.StepCompiledIn<UObjectProperty>(NULL);
		UObjectProperty* ContextProp = ExactCast<UObjectProperty>(Stack.MostRecentProperty);
		UObject* ContextValue = TargetProp->GetObjectPropertyValue(Stack.MostRecentPropertyAddress);
		
		P_FINISH;

		P_NATIVE_BEGIN;

		if(Target && ContextValue)
		{
			UE_LOG(LogTemp, Warning, TEXT("1 Valid Target and Context"));
			UFloatProperty* OutFloatProp = FindField<UFloatProperty>(Target->GetClass(), VariableName);
			if (OutFloatProp)
			{
				UE_LOG(LogTemp, Warning, TEXT("2 Valid float property, looking for value"));
				float OutFloat;
				UFloatProperty* FloatProp = FindField<UFloatProperty>(Target->GetClass(), VariableName);  // try to find float property in Target named VarName
				if (FloatProp) //if we found variable
				{
					UE_LOG(LogTemp, Warning, TEXT("3 found a float prop, confirming"));
					OutFloat = FloatProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp

					UE_LOG(LogTemp, Warning, TEXT("4 Context: %s"), *ContextValue->GetName());
					//ExactCast<UFloatProperty>(OutPropProp)->SetPropertyValue_InContainer(ContextValue, OutFloat);
					ExactCast<UFloatProperty>(OutPropProp)->SetPropertyValue_InContainer(ContextValue, OutFloat);
					*(bool*)RESULT_PARAM = true;
					return;
				}
			}

			UBoolProperty* OutBoolProp = FindField<UBoolProperty>(Target->GetClass(), VariableName);
			if (OutBoolProp)
			{
				UE_LOG(LogTemp, Warning, TEXT("2 Valid bool property, looking for value"));
				bool OutBool;
				UBoolProperty* BoolProp = FindField<UBoolProperty>(Target->GetClass(), VariableName);  // try to find bool property in Target named VarName
				if (BoolProp) //if we found variable
				{
					UE_LOG(LogTemp, Warning, TEXT("3 found a bool prop, confirming"));
					OutBool = BoolProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp

					UE_LOG(LogTemp, Warning, TEXT("4 Context: %s"), *ContextValue->GetName());
					ExactCast<UBoolProperty>(OutPropProp)->SetPropertyValue_InContainer(ContextValue, OutBool);
					*(bool*)RESULT_PARAM = true;
					return;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("5 Value Property Not Valid"));
			}
		}
		OutPropProp = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("6 Found nothing, fail"));

		*(bool*)RESULT_PARAM = false;
		
		P_NATIVE_END;
	}
	*/

};

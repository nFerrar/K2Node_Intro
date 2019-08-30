// Copyright Nicholas Ferrar 2019


#include "PSData.h"

#include "PSPluginSettings.h"

#include "ConfigCacheIni.h"


bool UPSData::DoesUseMiddleNames()
{
	bool Out = false;

	if (GConfig)
	{
		GConfig->GetBool(
			TEXT("/Script/nfPopulationSystem.PSPluginSettings"),
			TEXT("bUseMiddleNames"),
			Out,
			GGameIni
		);
	}
	
	return Out;
}

UDataTable* UPSData::GetFirstNamesData()
{
	UDataTable* Out = nullptr;
	FString AssetPath;

	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/nfPopulationSystem.PSPluginSettings"),
			TEXT("FirstNamesData"),
			AssetPath,
			GGameIni
		);

		FStringAssetReference AssetRef = FStringAssetReference(AssetPath);
		Out = Cast<UDataTable>(AssetRef.TryLoad());
	}

	return Out;
}

UDataTable* UPSData::GetMiddleNamesData()
{
	UDataTable* Out = nullptr;
	FString AssetPath;

	if(!DoesUseMiddleNames())
	{
		return nullptr;
	}
	else
	{
		if (GConfig)
		{
			GConfig->GetString(
				TEXT("/Script/nfPopulationSystem.PSPluginSettings"),
				TEXT("MiddleNamesData"),
				AssetPath,
				GGameIni
			);

			FStringAssetReference AssetRef = FStringAssetReference(AssetPath);
			Out = Cast<UDataTable>(AssetRef.TryLoad());
		}
	}

	return Out;
}

UDataTable* UPSData::GetLastNamesData()
{
	UDataTable* Out = nullptr;
	FString AssetPath;

	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/nfPopulationSystem.PSPluginSettings"),
			TEXT("LastNamesData"),
			AssetPath,
			GGameIni
		);

		FStringAssetReference AssetRef = FStringAssetReference(AssetPath);
		Out = Cast<UDataTable>(AssetRef.TryLoad());
	}

	return Out;
}

///end helpers///

///Getters and setters
//Setters

bool UPSData::SetFloatByName(UObject * Target, FName VarName, float NewValue, float & OutValue)
{
	if (Target)
	{
		float FoundValue;
		UFloatProperty* ValueProp = FindField<UFloatProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetIntByName(UObject * Target, FName VarName, int NewValue, int & OutValue)
{
	if (Target)
	{
		int FoundValue;
		UIntProperty* ValueProp = FindField<UIntProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetInt64ByName(UObject * Target, FName VarName, int64 NewValue, int64 & OutValue)
{
	//possibly not working
	if (Target)
	{
		int64 FoundValue;
		UUInt64Property* ValueProp = FindField<UUInt64Property>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetBoolByName(UObject * Target, FName VarName, bool NewValue, bool & OutValue)
{
	if (Target)
	{
		bool FoundValue;
		UBoolProperty* ValueProp = FindField<UBoolProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetNameByName(UObject * Target, FName VarName, FName NewValue, FName & OutValue)
{
	if (Target)
	{
		FName FoundValue;
		UNameProperty* ValueProp = FindField<UNameProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetObjectByName(UObject * Target, FName VarName, UObject* NewValue, UObject* & OutValue)
{
	if (Target)
	{
		UObject* FoundValue = nullptr;
		UObjectProperty* ValueProp = FindField<UObjectProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetClassByName(UObject * Target, FName VarName, class UClass* NewValue, class UClass* & OutValue)
{
	//Probably some weirdness to do here
	/*
	if (Target)
	{
		class UClass FoundValue;
		UClassProperty* ValueProp = FindField<UClassProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	*/
	return false;
}

bool UPSData::SetByteByName(UObject * Target, FName VarName, uint8 NewValue, uint8 & OutValue)
{
	if (Target)
	{
		uint8 FoundValue;
		UByteProperty* ValueProp = FindField<UByteProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetStringByName(UObject * Target, FName VarName, FString NewValue, FString & OutValue)
{
	if (Target)
	{
		FString FoundValue;
		UStrProperty* ValueProp = FindField<UStrProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetTextByName(UObject * Target, FName VarName, FText NewValue, FText & OutValue)
{
	if (Target)
	{
		FText FoundValue;
		UTextProperty* ValueProp = FindField<UTextProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	return false;
}

bool UPSData::SetStructByName(UObject * Target, FName VarName, UScriptStruct* NewValue, UScriptStruct* & OutValue)
{
	//This will need to iterate and shit
	/*
	if (Target)
	{
		UScriptStruct* FoundValue;
		UStructProperty* ValueProp = FindField<UStructProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	*/
	return false;
}

bool UPSData::SetEnumByName(UObject * Target, FName VarName, uint8 NewValue, uint8 & OutValue)
{
	//Some kind of magic here i guess
	/*
	if (Target)
	{
		uint8 FoundValue;
		UEnumProperty* ValueProp = FindField<UEnumProperty>(Target->GetClass(), VarName);
		if (ValueProp)
		{
			ValueProp->SetPropertyValue_InContainer(Target, NewValue); //this actually sets the variable
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);
			OutValue = FoundValue;
			return true;
		}
	}
	*/
	return false;
}

//Getters

bool UPSData::GetFloatByName(UObject * Target, FName VarName, float & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		float FoundValue;
		UFloatProperty* ValueProp = FindField<UFloatProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetIntByName(UObject * Target, FName VarName, int & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		int FoundValue;
		UIntProperty* ValueProp = FindField<UIntProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetInt64ByName(UObject * Target, FName VarName, int64 & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		int64 FoundValue;
		UInt64Property* ValueProp = FindField<UInt64Property>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetBoolByName(UObject * Target, FName VarName, bool &OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		bool FoundValue;
		UBoolProperty* ValueProp = FindField<UBoolProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetNameByName(UObject * Target, FName VarName, FName & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		FName FoundValue;
		UNameProperty* ValueProp = FindField<UNameProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetObjectByName(UObject * Target, FName VarName, UObject *& OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		UObject* FoundValue;
		UObjectProperty* ValueProp = FindField<UObjectProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetClassByName(UObject * Target, FName VarName, UClass *& OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		UClass* FoundValue;
		UClassProperty* ValueProp = FindField<UClassProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target)->StaticClass();  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetByteByName(UObject * Target, FName VarName, uint8 & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		uint8 FoundValue;
		UInt8Property* ValueProp = FindField<UInt8Property>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetStringByName(UObject * Target, FName VarName, FString & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		FString FoundValue;
		UStrProperty* ValueProp = FindField<UStrProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetTextByName(UObject * Target, FName VarName, FText & OutValue)
{
	if (Target) //make sure Target was set in blueprints. 
	{
		FText FoundValue;
		UTextProperty* ValueProp = FindField<UTextProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	return false; // we haven't found variable return false
}

bool UPSData::GetStructByName(UObject * Target, FName VarName, UScriptStruct *& OutValue)
{
	/*
	if (Target) //make sure Target was set in blueprints. 
	{
		UScriptStruct* FoundValue;
		UStructProperty* ValueProp = FindField<UStructProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	*/
	return false; // we haven't found variable return false
}

bool UPSData::GetEnumByName(UObject * Target, FName VarName, uint8 & OutValue)
{
	/*
	if (Target) //make sure Target was set in blueprints. 
	{
		float FoundValue;
		UEnumProperty* ValueProp = FindField<UEnumProperty>(Target->GetClass(), VarName);  // try to find float property in Target named VarName
		if (ValueProp) //if we found variable
		{
			FoundValue = ValueProp->GetPropertyValue_InContainer(Target);  // get the value from FloatProp
			OutValue = FoundValue;  // return float
			return true; // we can return
		}
	}
	*/
	return false; // we haven't found variable return false
}
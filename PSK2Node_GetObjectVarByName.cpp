// Copyright Nicholas Ferrar 2019


#include "PSK2Node_GetObjectVarByName.h"
#include "PSData.h"
#include "EdGraphSchema_K2.h"

#include "EdGraphUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_PureAssignmentStatement.h"
#include "K2Node_TemporaryVariable.h"

#include "KismetCompiler.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"

#define LOCTEXT_NAMESPACE "PSK2Node_GetObjectVarByName"

struct FGetGetterPinName
{

	static const FName& GetTargetPinName()
	{
		static const FName TargetPinName(TEXT("Target"));
		return TargetPinName;
	}

	static const FName& GetVarNamePinName()
	{
		static const FName VarNamePinName(TEXT("VarName"));
		return VarNamePinName;
	}

	static const FName& GetOutputResultPinName()
	{
		static const FName OutputResultPinName(TEXT("bSuccess"));
		return OutputResultPinName;
	}

	static const FName& GetOutputValuePinName()
	{
		static const FName OutputValuePinName(TEXT("FoundValue"));
		return OutputValuePinName;
	}
};

namespace FSetterFunctionNames
{
	static const FName FloatGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetFloatByName));
	static const FName IntGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetIntByName));
	static const FName Int64GetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetInt64ByName));
	static const FName BoolGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetBoolByName));
	static const FName NameGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetNameByName));
	static const FName ObjectGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetObjectByName));
	static const FName ClassGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetClassByName));
	static const FName ByteGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetByteByName));
	static const FName StringGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetStringByName));
	static const FName TextGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetTextByName));
	static const FName StructGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetStructByName));
	static const FName EnumGetterName(GET_FUNCTION_NAME_CHECKED(UPSData, GetEnumByName));
};

namespace
{
	// Optional pin manager subclass.
	struct FGetObjectVarOptionalPinManager : public FOptionalPinManager
	{
		FGetObjectVarOptionalPinManager(UClass* InClass, bool bExcludeObjectContainers, bool bExcludeObjectArrays)
			:FOptionalPinManager()
		{
			SrcClass = InClass;
			bExcludeObjectArrayProperties = bExcludeObjectContainers | bExcludeObjectArrays;
			bExcludeObjectContainerProperties = bExcludeObjectContainers;
		}

		virtual void GetRecordDefaults(UProperty* TestProperty, FOptionalPinFromProperty& Record) const override
		{
			FOptionalPinManager::GetRecordDefaults(TestProperty, Record);

			// Show pin unless the property is owned by a parent class.
			Record.bShowPin = TestProperty->GetOwnerClass() == SrcClass;
		}

		virtual bool CanTreatPropertyAsOptional(UProperty* TestProperty) const override
		{
			// Don't expose anything not marked BlueprintReadOnly/BlueprintReadWrite.
			if (!TestProperty || !TestProperty->HasAllPropertyFlags(CPF_BlueprintVisible))
			{
				return false;
			}

			if (UArrayProperty* TestArrayProperty = Cast<UArrayProperty>(TestProperty))
			{
				// We only use the Inner type if the flag is set. This is done for backwards-compatibility (some BPs may already rely on the previous behavior when the property value was allowed to be exposed).
				if (bExcludeObjectArrayProperties && TestArrayProperty->Inner)
				{
					TestProperty = TestArrayProperty->Inner;
				}
			}
			else if (USetProperty* TestSetProperty = Cast<USetProperty>(TestProperty))
			{
				if (bExcludeObjectContainerProperties && TestSetProperty->ElementProp)
				{
					TestProperty = TestSetProperty->ElementProp;
				}
			}
			else if (UMapProperty* TestMapProperty = Cast<UMapProperty>(TestProperty))
			{
				// Since we can't treat the key or value as read-only right now, we exclude any TMap that has a non-class UObject reference as its key or value type.
				return !(bExcludeObjectContainerProperties
					&& ((TestMapProperty->KeyProp && TestMapProperty->KeyProp->IsA<UObjectProperty>() && !TestMapProperty->KeyProp->IsA<UClassProperty>())
						|| (TestMapProperty->ValueProp && TestMapProperty->ValueProp->IsA<UObjectProperty>() && !TestMapProperty->ValueProp->IsA<UClassProperty>())));
			}

			// Don't expose object properties (except for those containing class objects).
			// @TODO - Could potentially expose object reference values if/when we have support for 'const' input pins.
			return !TestProperty->IsA<UObjectProperty>() || TestProperty->IsA<UClassProperty>();
		}

		virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex, UProperty* Property = nullptr) const override
		{
			check(Pin);

			// Move into the advanced view if the property metadata is set.
			Pin->bAdvancedView = Property && Property->HasAnyPropertyFlags(CPF_AdvancedDisplay);
		}

	private:
		// Class type for which optional pins are being managed.
		UClass* SrcClass;

		// Indicates whether or not object array properties will be excluded (for backwards-compatibility).
		bool bExcludeObjectArrayProperties;

		// Indicates whether or not object container properties will be excluded (will supercede the array-specific flag when true).
		bool bExcludeObjectContainerProperties;
	};

	// Compilation handler subclass.
	class FKCHandler_GetObjectVar : public FNodeHandlingFunctor
	{
	public:
		FKCHandler_GetObjectVar(FKismetCompilerContext& InCompilerContext)
			: FNodeHandlingFunctor(InCompilerContext)
		{
		}

		virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
		{
			// Cast to the correct node type
			if (const UPSK2Node_GetObjectVarByName* GetObjectVarNode = Cast<UPSK2Node_GetObjectVarByName>(Node))
			{
				// Only if we have a valid class input pin
				if (UEdGraphPin* ObjectPin = GetObjectVarNode->GetTargetPin())
				{
					// Redirect to a linked pin if necessary
					UEdGraphPin* Net = FEdGraphUtilities::GetNetFromPin(ObjectPin);
					check(Net != nullptr);

					// Register a literal if necessary (e.g. there are no linked pins)
					if (ValidateAndRegisterNetIfLiteral(Context, Net))
					{
						// First check for a literal term in case one was created above
						FBPTerminal** FoundTerm = Context.LiteralHackMap.Find(Net);
						if (FoundTerm == nullptr)
						{
							// Otherwise, check for a linked term
							FoundTerm = Context.NetMap.Find(Net);
						}

						// If we did not find an input term, make sure we create one here
						FBPTerminal* ClassContextTerm = FoundTerm ? *FoundTerm : nullptr;
						if (ClassContextTerm == nullptr)
						{
							ClassContextTerm = Context.CreateLocalTerminalFromPinAutoChooseScope(Net, Context.NetNameMap->MakeValidName(Net));
							check(ClassContextTerm != nullptr);

							Context.NetMap.Add(Net, ClassContextTerm);
						}

						// Flag this as a "class context" term
						ClassContextTerm->SetContextTypeClass();

						// Infer the class type from the context term
						if (const UClass* ClassType = Cast<UClass>(ClassContextTerm->bIsLiteral ? ClassContextTerm->ObjectLiteral : ClassContextTerm->Type.PinSubCategoryObject.Get()))
						{
							// Create a local term for each output pin (class property)
							for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
							{
								UEdGraphPin* Pin = Node->Pins[PinIndex];
								if (Pin != nullptr && Pin->Direction == EGPD_Output)
								{
									UProperty* BoundProperty = FindField<UProperty>(ClassType, Pin->PinName);
									if (BoundProperty != nullptr)
									{
										FBPTerminal* OutputTerm = Context.CreateLocalTerminalFromPinAutoChooseScope(Pin, Pin->PinName.ToString());
										check(OutputTerm != nullptr);

										// Set as a variable within the class context
										OutputTerm->AssociatedVarProperty = BoundProperty;
										OutputTerm->Context = ClassContextTerm;

										// Flag this as a "class default" variable term
										OutputTerm->bIsConst = true;
										OutputTerm->SetVarTypeDefault();

										// Add it to the lookup table
										Context.NetMap.Add(Pin, OutputTerm);
									}
									else if(Pin != GetObjectVarNode->GetThenPin())
									{
										CompilerContext.MessageLog.Error(*LOCTEXT("UnmatchedOutputPinOnCompile", "Failed to find a class member to match @@").ToString(), Pin);
									}
								}
							}
						}
						else
						{
							CompilerContext.MessageLog.Error(*LOCTEXT("InvalidClassTypeOnCompile", "Missing or invalid input class type for @@").ToString(), Node);
						}
					}
				}
			}
		}
	};
}

//UObject interface
void UPSK2Node_GetObjectVarByName::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FOptionalPinFromProperty, bShowPin))
	{
		UpdateShowPinForProperties();
		FOptionalPinManager::EvaluateOldShownPins(ShowPinForProperties, OldShownPins, this);
		GetSchema()->ReconstructNode(*this);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UPSK2Node_GetObjectVarByName::PreEditChange(UProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

	if (PropertyThatWillChange && PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(FOptionalPinFromProperty, bShowPin))
	{
		UpdateShowPinForProperties();
		FOptionalPinManager::CacheShownPins(ShowPinForProperties, OldShownPins);
	}
}
//end uobject interface

void UPSK2Node_GetObjectVarByName::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	/*Create our pins*/

	// Execution pins
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	//Input
	UEdGraphNode::FCreatePinParams PinParams;
	PinParams.bIsReference = true;

	UEdGraphPin* InTargetPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), FGetGetterPinName::GetTargetPinName(), PinParams);
	K2Schema->ConstructBasicPinTooltip(*InTargetPin, LOCTEXT("TargetPinDescription", "The Object we want to get the variable from."), InTargetPin->PinToolTip);
	UEdGraphPin* InVarNamePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, FGetGetterPinName::GetVarNamePinName());
	K2Schema->SetPinAutogeneratedDefaultValue(InVarNamePin, FName("MyVarName").ToString());
	K2Schema->ConstructBasicPinTooltip(*InVarNamePin, LOCTEXT("VarNamePinDescription", "The name of the variable we want to get."), InVarNamePin->PinToolTip);


	//Output
	UEdGraphPin* OutValidPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FGetGetterPinName::GetOutputResultPinName());
	K2Schema->SetPinAutogeneratedDefaultValueBasedOnType(OutValidPin);
	/*
	UEdGraphPin* OutNewValuePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FGetGetterPinName::GetOutputValuePinName());
	*/
	Super::AllocateDefaultPins();
}

void UPSK2Node_GetObjectVarByName::PostPlacedNewNode()
{
	// Always exclude object container properties for new nodes.
	// @TODO - Could potentially expose object reference values if/when we have support for 'const' input pins.
	bExcludeObjectContainers = true;

	//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[0]: Run."));

	//if (UEdGraphPin* ClassPin = FindTargetPin(Pins))
	UEdGraphPin* TargetPin = GetTargetPin();
	if(TargetPin)
	{
		//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[1]: Found Target Pin"));
		// Default to the owner BP's generated class for "normal" BPs if this is a new node
		const UBlueprint* OwnerBlueprint = GetBlueprint();
		//if (OwnerBlueprint != nullptr && OwnerBlueprint->BlueprintType == BPTYPE_Normal)
		if(OwnerBlueprint != nullptr)
		{
			//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[1.1]: Owner Exists"));
			if (OwnerBlueprint->BlueprintType == BPTYPE_Normal)
			{
				//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[1.2]: Type is Normal"));
				TargetPin->DefaultObject = OwnerBlueprint->GeneratedClass;
				//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[2]: ClassPin Object is OwnerBP class"));
			}
		}

		if (UClass* InputClass = GetInputClass(TargetPin))
		{
			//UE_LOG(LogTemp, Warning, TEXT("PostPlacedNewNode[3]: Creating Output Pins."));
			CreateOutputPins(InputClass);
		}
	}
}

void UPSK2Node_GetObjectVarByName::PinDefaultValueChanged(UEdGraphPin * Pin)
{
	if (Pin != nullptr && Pin->PinName == FGetGetterPinName::GetTargetPinName() && Pin->Direction == EGPD_Input)
	{
		OnClassPinChanged();
	}
	else if (Pin != nullptr && Pin->PinName == FGetGetterPinName::GetVarNamePinName() && Pin->Direction == EGPD_Input)
	{
		OnVarNamePinChanged();
	}
	/*
	if (Pin)
	{

		
		if (Pin->PinName == FGetGetterPinName::GetOutputValuePinName())
		{
			GetReturnValuePin()->PinType = Pin->PinType;
		}
		
	}
	*/
}

void UPSK2Node_GetObjectVarByName::PinConnectionListChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin != nullptr && ChangedPin->PinName == FGetGetterPinName::GetTargetPinName() && ChangedPin->Direction == EGPD_Input)
	{
		OnClassPinChanged();
	}
}

///These three functions make the node appear as an option;

FText UPSK2Node_GetObjectVarByName::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("GetObjVarByNameK2Node_Title", "Get Object Variable By Name");
}

FText UPSK2Node_GetObjectVarByName::GetTooltipText() const
{
	return LOCTEXT("GetObjVarByNameK2Node_Tooltip", "Gets the value of a variable in a provided object. Takes in the target object and the name of the variable to be returned.");
}

FText UPSK2Node_GetObjectVarByName::GetMenuCategory() const
{
	return LOCTEXT("GetObjVarByNameK2Node_MenuCategory", "nfPopulationSystem");
}

///end

///K2 Implementation

void UPSK2Node_GetObjectVarByName::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	UEdGraphPin* OldClassPin = GetTargetPin();
	AllocateDefaultPins();

	// Recreate output pins based on the previous input class
	//UEdGraphPin* OldClassPin = FindClassPin(OldPins);
	if (UClass* InputClass = GetInputClass(OldClassPin))
	{
		CreateOutputPins(InputClass);
	}

	RestoreSplitPins(OldPins);
}

bool UPSK2Node_GetObjectVarByName::HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const
{
	UClass* SourceClass = GetInputClass();
	UBlueprint* SourceBlueprint = GetBlueprint();
	const bool bResult = (SourceClass && (SourceClass->ClassGeneratedBy != SourceBlueprint));
	if (bResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(SourceClass);
	}

	const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return bSuperResult || bResult;
}

FNodeHandlingFunctor* UPSK2Node_GetObjectVarByName::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FKCHandler_GetObjectVar(CompilerContext);
}

void UPSK2Node_GetObjectVarByName::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	//UE_LOG(LogTemp, Warning, TEXT("ExpandNode[0]: Run."));

	UFunction* BlueprintFunction = FindGetterFunctionByType(GetReturnValueType());

	if (!BlueprintFunction)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InvalidFunctionName", "The function has not been found.").ToString(), this);
		return;
	}

	const UClass* ClassType = GetInputClass();

	// @TODO - Remove if/when we support 'const' input pins.
	// For container properties, return a local copy of the container so that the original cannot be modified.
	for (UEdGraphPin* OutputPin : Pins)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ExpandNode[1]: For outputPins."));
		if (OutputPin != nullptr && OutputPin->Direction == EGPD_Output && OutputPin->LinkedTo.Num() > 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("ExpandNode[2]: OutputPin valid && OutputPin dir->out && OutputPin linked to > 0."));
			UProperty* BoundProperty = FindField<UProperty>(ClassType, OutputPin->PinName);
			if (BoundProperty != nullptr && (BoundProperty->IsA<UArrayProperty>() || BoundProperty->IsA<USetProperty>() || BoundProperty->IsA<UMapProperty>()))
			{
				//UE_LOG(LogTemp, Warning, TEXT("ExpandNode[3]: BoundProp valid &&  is either set, array, or map."));
				UK2Node_TemporaryVariable* LocalVariable = CompilerContext.SpawnIntermediateNode<UK2Node_TemporaryVariable>(this, SourceGraph);
				LocalVariable->VariableType = OutputPin->PinType;
				LocalVariable->VariableType.bIsReference = false;
				LocalVariable->AllocateDefaultPins();

				UK2Node_PureAssignmentStatement* CopyDefaultValue = CompilerContext.SpawnIntermediateNode<UK2Node_PureAssignmentStatement>(this, SourceGraph);
				CopyDefaultValue->AllocateDefaultPins();
				CopyDefaultValue->GetVariablePin()->PinType = OutputPin->PinType;
				CompilerContext.GetSchema()->TryCreateConnection(LocalVariable->GetVariablePin(), CopyDefaultValue->GetVariablePin());

				// Note: This must be done AFTER connecting the variable input, which sets the pin type.
				CompilerContext.MovePinLinksToIntermediate(*OutputPin, *CopyDefaultValue->GetOutputPin());
				CompilerContext.GetSchema()->TryCreateConnection(OutputPin, CopyDefaultValue->GetValuePin());
			}
		}
	}

	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	//UE_LOG(LogTemp, Warning, TEXT("ExpandNode[4]: Setting up function call."));

	CallFunction->SetFromFunction(BlueprintFunction);
	CallFunction->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	//Exec pins
	UEdGraphPin* NodeExec = GetExecPin();
	UEdGraphPin* NodeThen = FindPin(UEdGraphSchema_K2::PN_Then);

	UEdGraphPin* InternalExec = CallFunction->GetExecPin();
	CompilerContext.MovePinLinksToIntermediate(*NodeExec, *InternalExec);

	UEdGraphPin* InternalThen = CallFunction->GetThenPin();
	CompilerContext.MovePinLinksToIntermediate(*NodeThen, *InternalThen);

	//Input
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetTargetPinName()), *CallFunction->FindPinChecked(TEXT("Target")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetVarNamePinName()), *CallFunction->FindPinChecked(TEXT("VarName")));

	//Output
	//CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetOutputValuePinName()), *CallFunction->FindPinChecked(TEXT("OutValue")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(GetReturnValuePin()->PinName), *CallFunction->FindPinChecked(TEXT("OutValue")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetOutputResultPinName()), *CallFunction->GetReturnValuePin());

	//After we are done we break all links to this node (not the internally created one)
	BreakAllNodeLinks();

	/*
	UFunction* BlueprintFunction = FindGetterFunctionByType(GetReturnValuePin()->PinType);

	if (!BlueprintFunction)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InvalidFunctionName", "The function has not been found.").ToString(), this);
		return;
	}

	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

	CallFunction->SetFromFunction(BlueprintFunction);
	CallFunction->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	//Input
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetTargetPinName()), *CallFunction->FindPinChecked(TEXT("Target")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetVarNamePinName()), *CallFunction->FindPinChecked(TEXT("VarName")));

	//Output
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetOutputValuePinName()), *CallFunction->FindPinChecked(TEXT("OutValue")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(FGetGetterPinName::GetOutputResultPinName()), *CallFunction->GetReturnValuePin());

	//Exec pins
	UEdGraphPin* NodeExec = GetExecPin();
	UEdGraphPin* NodeThen = FindPin(UEdGraphSchema_K2::PN_Then);

	UEdGraphPin* InternalExec = CallFunction->GetExecPin();
	CompilerContext.MovePinLinksToIntermediate(*NodeExec, *InternalExec);

	UEdGraphPin* InternalThen = CallFunction->GetThenPin();
	CompilerContext.MovePinLinksToIntermediate(*NodeThen, *InternalThen);
	
	//After we are done we break all links to this node (not the internally created one)
	BreakAllNodeLinks();
	*/
}

//This method adds our node to the context menu
void UPSK2Node_GetObjectVarByName::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);

	UClass* Action = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(Action))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		check(Spawner != nullptr);

		ActionRegistrar.AddBlueprintAction(Action, Spawner);
	}
}

void UPSK2Node_GetObjectVarByName::NotifyPinConnectionListChanged(UEdGraphPin * Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	/*
	UEdGraphPin* OutValuePin = GetReturnValuePin();

	if(Pin != OutValuePin)
	{
		UEdGraphPin* ConnectedToPin = (Pin->LinkedTo.Num() > 0) ? Pin->LinkedTo[0] : NULL;
		CoerceTypeForPin(ConnectedToPin);
	}
	*/
}

void UPSK2Node_GetObjectVarByName::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	if (const UClass* SourceClass = GetInputClass())
	{
		for (const UEdGraphPin* Pin : Pins)
		{
			// Emit a warning for existing connections to potentially unsafe array property defaults. We do this rather than just implicitly breaking the connection (for compatibility).
			if (Pin && Pin->Direction == EGPD_Output && Pin->LinkedTo.Num() > 0)
			{
				// Even though container property defaults are copied, the copy could still contain a reference to a non-class object that belongs to the CDO, which would potentially be unsafe to modify.
				bool bEmitWarning = false;
				const UProperty* TestProperty = SourceClass->FindPropertyByName(Pin->PinName);
				if (const UArrayProperty* ArrayProperty = Cast<UArrayProperty>(TestProperty))
				{
					bEmitWarning = ArrayProperty->Inner && ArrayProperty->Inner->IsA<UObjectProperty>() && !ArrayProperty->Inner->IsA<UClassProperty>();
				}
				else if (const USetProperty* SetProperty = Cast<USetProperty>(TestProperty))
				{
					bEmitWarning = SetProperty->ElementProp && SetProperty->ElementProp->IsA<UObjectProperty>() && !SetProperty->ElementProp->IsA<UClassProperty>();
				}
				else if (const UMapProperty* MapProperty = Cast<UMapProperty>(TestProperty))
				{
					bEmitWarning = (MapProperty->KeyProp && MapProperty->KeyProp->IsA<UObjectProperty>() && !MapProperty->KeyProp->IsA<UClassProperty>())
						|| (MapProperty->ValueProp && MapProperty->ValueProp->IsA<UObjectProperty>() && !MapProperty->ValueProp->IsA<UClassProperty>());
				}

				if (bEmitWarning)
				{
					MessageLog.Warning(*LOCTEXT("UnsafeConnectionWarning", "@@ has an unsafe connection to the @@ output pin that is not fully supported at this time. It should be disconnected to avoid potentially corrupting class defaults at runtime. If you need to keep this connection, make sure you're not changing the state of any elements in the container. Also note that if you recreate this node, it will not include this output pin.").ToString(), this, Pin);
				}
			}
		}
	}
}

///Pin getters

UEdGraphPin * UPSK2Node_GetObjectVarByName::GetThenPin() const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin * UPSK2Node_GetObjectVarByName::GetTargetPin() const
{
	UEdGraphPin* Pin = FindPin(FGetGetterPinName::GetTargetPinName());
	check(Pin == NULL || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin * UPSK2Node_GetObjectVarByName::GetVarNamePin() const
{
	UEdGraphPin* Pin = FindPin(FGetGetterPinName::GetVarNamePinName());
	check(Pin == NULL || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin * UPSK2Node_GetObjectVarByName::GetReturnResultPin() const
{
	UEdGraphPin* Pin = FindPin(FGetGetterPinName::GetOutputResultPinName());
	check(Pin == NULL || Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin * UPSK2Node_GetObjectVarByName::GetReturnValuePin() const
{
	if (Pins.Num() > 5)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GetReturnValuePin[0]: Run."));
		UEdGraphPin* Pin = FindPin(GetVarNamePin()->DefaultValue);
		//UE_LOG(LogTemp, Warning, TEXT("GetReturnValuePin[0.1]: %s"), *GetVarNamePin()->DefaultValue);
		check(Pin == NULL || Pin->Direction == EGPD_Output);
		return Pin;
	}
	else
	{
		return nullptr;
	}
}

FEdGraphPinType& UPSK2Node_GetObjectVarByName::GetReturnValueType() const
{
	if (Pins.Num() > 5)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GetReturnValueType[0]: Run."));
		//UEdGraphPin* Pin = FindPin(FGetGetterPinName::GetOutputValuePinName());
		UEdGraphPin* Pin = FindPin(GetVarNamePin()->DefaultValue);
		//UE_LOG(LogTemp, Warning, TEXT("GetReturnValueType[0.1]: %s"), *GetVarNamePin()->DefaultValue);
		check(Pin == NULL || Pin->Direction == EGPD_Output);
		//UE_LOG(LogTemp, Warning, TEXT("GetReturnValueType[1]: %s."), *Pin->PinType.PinCategory.ToString());
		return Pin->PinType;
	}
	else
	{
		FEdGraphPinType* out = new FEdGraphPinType();
		return *out;
	}
}

///Node type handling
/*
void UPSK2Node_GetObjectVarByName::CoerceTypeForPin(const UEdGraphPin * Pin)
{
	UEdGraphPin* OutValuePin = GetReturnValuePin();

	UEdGraphPin* InTargetPin = GetTargetPin();
	UEdGraphPin* InVarNamePin = GetVarNamePin();

	check(OutValuePin);
	check(InTargetPin);
	check(InVarNamePin);

	//pin was connected to something, do the calc
	if (Pin && InTargetPin->LinkedTo.Num() > 0 && InVarNamePin->DefaultTextValue.IsEmpty())
	{
		///shit is fucked here. All wrong
		//trying to find the desired output pin type at compile rather than runtime.
		//Probably will have to add that compile event
	}
	else
	{
		// Pin disconnected...revert to wildcard
		OutValuePin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
		OutValuePin->PinType.PinSubCategory = NAME_None;
		OutValuePin->PinType.PinSubCategoryObject = nullptr;
		OutValuePin->BreakAllPinLinks();

	}
}
*/
///Finders

//find setter
UFunction * UPSK2Node_GetObjectVarByName::FindGetterFunctionByType(FEdGraphPinType& PinType)
{
	UClass* LibraryClass = UPSData::StaticClass();
	FName FunctionName = NAME_None;
	UFunction* Function = nullptr;

	//this should really be in a const FNAME
	//here goes a bunch of IFs to check the pin type and set FUNCTION NAME to its NAME
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Float)
	{
		FunctionName = FSetterFunctionNames::FloatGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
	{
		FunctionName = FSetterFunctionNames::IntGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Int64)
	{
		FunctionName = FSetterFunctionNames::Int64GetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean)
	{
		FunctionName = FSetterFunctionNames::BoolGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Name)
	{
		FunctionName = FSetterFunctionNames::NameGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Object)
	{
		FunctionName = FSetterFunctionNames::ObjectGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Class)
	{
		FunctionName = FSetterFunctionNames::ClassGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Byte)
	{
		FunctionName = FSetterFunctionNames::ByteGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_String)
	{
		FunctionName = FSetterFunctionNames::StringGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Text)
	{
		FunctionName = FSetterFunctionNames::TextGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
	{
		FunctionName = FSetterFunctionNames::StructGetterName;
	}
	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Enum)
	{
		FunctionName = FSetterFunctionNames::EnumGetterName;
	}

	if (!FunctionName.IsNone())
	{
		Function = LibraryClass->FindFunctionByName(FunctionName);
	}

	return Function;
}

///Protected

UClass* UPSK2Node_GetObjectVarByName::GetInputClass(const UEdGraphPin* FromPin) const
{
	UClass* InputClass = nullptr;

	//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[0]: Run."));
	if (FromPin != nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[1]: FromPin valid."));
		check(FromPin->Direction == EGPD_Input);

		if (FromPin->DefaultObject != nullptr && FromPin->LinkedTo.Num() == 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[2]: DefaultObject valid && Linked to 0."));
			InputClass = CastChecked<UClass>(FromPin->DefaultObject);
		}
		else if (FromPin->LinkedTo.Num() > 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[3]: FromPin linked to > 0."));
			if (UEdGraphPin* LinkedPin = FromPin->LinkedTo[0])
			{
				//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[4]: LinkedTo[0] valid."));
				InputClass = Cast<UClass>(LinkedPin->PinType.PinSubCategoryObject.Get());
			}
		}
	}

	// Switch Blueprint Class types to use the generated skeleton class (if valid).
	if (InputClass)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[5]: InputClass Valid."));
		if (UBlueprint* Blueprint = Cast<UBlueprint>(InputClass->ClassGeneratedBy))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetInputClass[6]: Blueprint made from input class."));
			// Stick with the original (serialized) class if the skeleton class is not valid for some reason (e.g. the Blueprint hasn't been compiled on load yet).
			// Note: There's not a need to force it to be preloaded here in that case, because once it is loaded, we'll end up reconstructing this node again anyway.
			if (Blueprint->SkeletonGeneratedClass)
			{
				////UE_LOG(LogTemp, Warning, TEXT("GetInputClass[7]: skeletonClass valid."));
				InputClass = Blueprint->SkeletonGeneratedClass;
			}
		}
	}

	return InputClass;
}

void UPSK2Node_GetObjectVarByName::OnBlueprintClassModified(UBlueprint* TargetBlueprint)
{
	check(TargetBlueprint);
	UBlueprint* OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForNode(this); //GetBlueprint() will crash, when the node is transient, etc
	if (OwnerBlueprint)
	{
		// The Blueprint that contains this node may have finished 
		// regenerating (see bHasBeenRegenerated), but we still may be
		// in the midst of unwinding a cyclic load (dependent Blueprints);
		// this lambda could be triggered during the targeted 
		// Blueprint's regeneration - meaning we really haven't completed 
		// the load process. In this situation, we cannot "reset loaders" 
		// because it is not likely that all of the package's objects
		// have been post-loaded (meaning an assert will most likely  
		// fire from ReconstructNode). To guard against this, we flip this
		// Blueprint's bIsRegeneratingOnLoad (like in 
		// UBlueprintGeneratedClass::ConditionalRecompileClass), which
		// we use throughout Blueprints to keep us from reseting loaders 
		// on object Rename()
		const bool bOldIsRegeneratingVal = OwnerBlueprint->bIsRegeneratingOnLoad;
		OwnerBlueprint->bIsRegeneratingOnLoad = bOldIsRegeneratingVal || TargetBlueprint->bIsRegeneratingOnLoad;

		ReconstructNode();

		OwnerBlueprint->bIsRegeneratingOnLoad = bOldIsRegeneratingVal;
	}
}

void UPSK2Node_GetObjectVarByName::CreateOutputPins(UClass* InClass)
{
	//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[0]: Run."));
	// Create the set of output pins through the optional pin manager
	//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[0.1]: Total Outputs to create: %s"), *FString::FromInt(ShowPinForProperties.Num()));
	FGetObjectVarOptionalPinManager OptionalPinManager(InClass, bExcludeObjectContainers, bExcludeObjectArrays_DEPRECATED);
	OptionalPinManager.RebuildPropertyList(ShowPinForProperties, InClass);
	UpdateShowPinForProperties();
	//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[0.2]: Total Outputs to create: %s"), *FString::FromInt(ShowPinForProperties.Num()));
	
	//I added
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* OutValidPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FGetGetterPinName::GetOutputResultPinName());
	//end

	if (CheckVarExists(ShowPinForProperties[0], InClass))
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[0.3]: Var exists."));
		OptionalPinManager.CreateVisiblePins(ShowPinForProperties, InClass, EGPD_Output, this);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[0.4]: Var does not exist."));
		return;
	}

	// Check for any advanced properties (outputs)
	bool bHasAdvancedPins = false;
	for (int32 PinIndex = 0; PinIndex < Pins.Num() && !bHasAdvancedPins; ++PinIndex)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[1]: For Pins.Num()."));
		UEdGraphPin* Pin = Pins[PinIndex];
		check(Pin != nullptr);

		bHasAdvancedPins |= Pin->bAdvancedView;
	}

	// Toggle advanced display on/off based on whether or not we have any advanced outputs
	if (bHasAdvancedPins && AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[2]: HasAdvanced && NoPins."));
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	}
	else if (!bHasAdvancedPins)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[4]: !HasAdvanced."));
		AdvancedPinDisplay = ENodeAdvancedPins::NoPins;
	}

	// Unbind OnChanged() delegate from a previous Blueprint, if valid.

	// If the class was generated for a Blueprint, bind delegates to handle any OnChanged() & OnCompiled() events.
	bool bShouldClearDelegate = true;
	if (InClass)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[5]: InClass valid."));
		if (UBlueprint* Blueprint = Cast<UBlueprint>(InClass->ClassGeneratedBy))
		{
			//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[7]: Blueprint from InClass."));
			// only clear the delegate if the pin has changed:
			bShouldClearDelegate = BlueprintSubscribedTo != Blueprint;
		}
	}

	if (bShouldClearDelegate)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[8]: ShouldClearDelegate."));
		if (OnBlueprintChangedDelegate.IsValid())
		{
			//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[9]: OnBlueprintChangedDelegate valid."));
			if (BlueprintSubscribedTo)
			{
				//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[10]: BlueprintSubscribedTo."));
				BlueprintSubscribedTo->OnChanged().Remove(OnBlueprintChangedDelegate);
			}
			OnBlueprintChangedDelegate.Reset();
		}

		// Unbind OnCompiled() delegate from a previous Blueprint, if valid.
		if (OnBlueprintCompiledDelegate.IsValid())
		{
			//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[11]: OnBklueprintCompiledDelegate is valid."));
			if (BlueprintSubscribedTo)
			{
				//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[12]: BlueprintSubscribedTo."));
				BlueprintSubscribedTo->OnCompiled().Remove(OnBlueprintCompiledDelegate);
			}
			OnBlueprintCompiledDelegate.Reset();
		}
		// Associated Blueprint changed, clear the BlueprintSubscribedTo:
		BlueprintSubscribedTo = nullptr;
	}

	if (InClass && bShouldClearDelegate)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[13]: InClass && bShouldClearDelegate."));
		if (UBlueprint* Blueprint = Cast<UBlueprint>(InClass->ClassGeneratedBy))
		{
			//UE_LOG(LogTemp, Warning, TEXT("CreateOutputPins[14]: Blueprint from InClass."));
			BlueprintSubscribedTo = Blueprint;
			OnBlueprintChangedDelegate = Blueprint->OnChanged().AddUObject(this, &ThisClass::OnBlueprintClassModified);
			OnBlueprintCompiledDelegate = Blueprint->OnCompiled().AddUObject(this, &ThisClass::OnBlueprintClassModified);
		}
	}
}

void UPSK2Node_GetObjectVarByName::OnClassPinChanged()
{
	Modify();

	TArray<UEdGraphPin*> OldOutputPins = MoveTemp(Pins);

	// Gather all current output pins
	for (int32 PinIndex = 0; PinIndex < OldOutputPins.Num(); ++PinIndex)
	{
		UEdGraphPin* OldPin = OldOutputPins[PinIndex];
		if (OldPin->Direction == EGPD_Input)
		{
			OldOutputPins.RemoveAt(PinIndex--, 1, false);
			Pins.Add(OldPin);
		}
	}

	// Clear the current output pin settings (so they don't carry over to the new set)
	ShowPinForProperties.Reset();

	// Create output pins for the new class type
	UClass* InputClass = GetInputClass();
	CreateOutputPins(InputClass);

	// Restore the split pins and reconnect output pins to previously linked inputs
	RestoreSplitPins(OldOutputPins);
	RewireOldPinsToNewPins(OldOutputPins, Pins, nullptr);

	// Notify the graph that the node has been changed
	if (UEdGraph* Graph = GetGraph())
	{
		Graph->NotifyGraphChanged();
	}
}

void UPSK2Node_GetObjectVarByName::OnVarNamePinChanged()
{
	Modify();

	if (GetReturnValuePin() != nullptr && GetReturnValuePin()->LinkedTo.Num() > 0)
	{
		GetReturnValuePin()->BreakAllPinLinks();
	}

	OnClassPinChanged();
}

void UPSK2Node_GetObjectVarByName::UpdateShowPinForProperties()
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateShowPinForProperties[0]: Run."));
	ShowPinForProperties.Empty();

	FOptionalPinFromProperty PinProps = FOptionalPinFromProperty();

	PinProps.PropertyName = *GetVarNamePin()->DefaultValue;
	PinProps.bShowPin = true;
	PinProps.bIsSetValuePinVisible = true;
	//UE_LOG(LogTemp, Warning, TEXT("UpdateShowPinForProperties[1]: Pin value %s."), *GetVarNamePin()->DefaultValue);
	ShowPinForProperties.Add(PinProps);
	//UE_LOG(LogTemp, Warning, TEXT("UpdateShowPinForProperties[2]: Length %s."), *FString::FromInt(ShowPinForProperties.Num()));
}

bool UPSK2Node_GetObjectVarByName::CheckVarExists(FOptionalPinFromProperty& Prop, UStruct * InClass)
{
	bool Out = false;

	UProperty* OuterProperty = nullptr;
	OuterProperty = FindField<UProperty>(InClass, Prop.PropertyName);
	if (OuterProperty)
	{
		Out = true;
	}

	return Out;
}

#undef LOCTEXT_NAMESPACE
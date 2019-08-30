// Copyright Nicholas Ferrar 2019

#pragma once

#include "CoreMinimal.h"

#include "UObject/ObjectMacros.h"

#include "K2Node.h"
#include "PSK2Node_GetObjectVarByName.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UBlueprint;
class UEdGraph;
class UEdGraphPin;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class NFPOPULATIONSYSTEMEDITOR_API UPSK2Node_GetObjectVarByName : public UK2Node
{
	GENERATED_BODY()

public:

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
	//~ End UObject Interface

	//UEdGraphNode implementation
	virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//UEdGraphNode implementation

	//K2Node implementation
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual bool HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const override;
	virtual class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	//K2Node implementation

	//Helpers for getting pins
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetTargetPin() const;
	UEdGraphPin* GetVarNamePin() const;
	UEdGraphPin* GetReturnResultPin() const;
	UEdGraphPin* GetReturnValuePin() const;
	FEdGraphPinType& GetReturnValueType() const;
	
	//Custom functions
	//void CoerceTypeForPin(const UEdGraphPin* Pin);

	static UFunction* FindGetterFunctionByType(FEdGraphPinType& PinType);

	/** Retrieves the current input class type. */
	/*BLUEPRINTGRAPH_API*/ UClass* GetInputClass() const
	{
		return GetInputClass(GetTargetPin());
	}

	//Custom From Get Class Defaults
	void OnBlueprintClassModified(UBlueprint* TargetBlueprint);

protected:

	/**
	* Determines the input class type from the given pin.
	*
	* @param FromPin	Input class pin.
	*/
	/*BLUEPRINTGRAPH_API*/ UClass* GetInputClass(const UEdGraphPin* FromPin) const;

	/**
	 * Creates the full set of output pins (properties) from the given input class.
	 *
	 * @param InClass	Input class type.
	 */
	void CreateOutputPins(UClass* InClass);

	/** Will be called whenever the class pin selector changes its value. */
	void OnClassPinChanged();

	void OnVarNamePinChanged();

private:

	/** Blueprint that we subscribed OnBlueprintChangedDelegate and OnBlueprintCompiledDelegate to */
	UPROPERTY()
		UBlueprint* BlueprintSubscribedTo;

	/** Blueprint.OnChanged delegate handle */
	FDelegateHandle OnBlueprintChangedDelegate;

	/** Blueprint.OnCompiled delegate handle */
	FDelegateHandle OnBlueprintCompiledDelegate;

	/** Output pin visibility control */
	UPROPERTY(EditAnywhere, Category = PinOptions, EditFixedSize)
		TArray<FOptionalPinFromProperty> ShowPinForProperties;

	TArray<FName> OldShownPins;

	/** Whether or not to exclude object container properties */
	UPROPERTY()
		bool bExcludeObjectContainers;

	/** Whether or not to exclude object array properties (deprecated) */
	UPROPERTY()
		bool bExcludeObjectArrays_DEPRECATED;

	void UpdateShowPinForProperties();
	bool CheckVarExists(FOptionalPinFromProperty& Prop, UStruct* InClass);
};

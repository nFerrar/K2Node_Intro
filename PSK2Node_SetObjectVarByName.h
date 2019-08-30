// Copyright Nicholas Ferrar 2019

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "PSK2Node_SetObjectVarByName.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class NFPOPULATIONSYSTEMEDITOR_API UPSK2Node_SetObjectVarByName : public UK2Node
{
	GENERATED_BODY()

public:

	//UEdGraphNode implementation
	virtual void AllocateDefaultPins() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//UEdGraphNode implementation

	//K2Node implementation
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	//K2Node implementation

	//Helpers for getting pins
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetTargetPin() const;
	UEdGraphPin* GetVarNamePin() const;
	UEdGraphPin* GetNewValuePin() const;
	UEdGraphPin* GetReturnResultPin() const;
	UEdGraphPin* GetReturnValuePin() const;
	
	//Custom functions
	void CoerceTypeFromPin(const UEdGraphPin* Pin, bool bWasNewValuePin);

	static UFunction* FindSetterFunctionByType(FEdGraphPinType& PinType);

};



#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ComboAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UComboAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GASPROJECT_API IComboAttackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category="Combo")
	void RemoveComboAttackBinding();
	
};



#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "CombatActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GASPROJECT_API ICombatActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CombatSystem")
	void OnAttack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CombatSystem")
	void OnHit();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CombatSystem")
	void OnDie();
};

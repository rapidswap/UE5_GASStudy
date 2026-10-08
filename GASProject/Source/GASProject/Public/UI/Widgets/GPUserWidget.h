// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GPUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API UGPUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGPUserWidget(const FObjectInitializer& ObjectInitializer);
	// Setter
	FORCEINLINE void SetOwningActor(AActor* NewOwner) { OwningActor = NewOwner; }
	

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Actor")
	TObjectPtr<AActor> OwningActor;
};

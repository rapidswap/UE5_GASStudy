// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/GPHpBarWidget.h"
#include "GPPlayerStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API UGPPlayerStatWidget : public UGPHpBarWidget
{
	GENERATED_BODY()

public:
	UGPPlayerStatWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void UpdateStatBar(float CurrentStamina) override;

	FORCEINLINE void SetMaxStamina(float NewMaxStamina) { MaxStamina = NewMaxStamina; }
protected:
	UPROPERTY()
	float MaxStamina;

	UPROPERTY()
	TObjectPtr<class UProgressBar> StaminaProgressBar;
	
};

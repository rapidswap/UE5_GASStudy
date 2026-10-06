// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerVitalsWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;
/**
 * 
 */
UCLASS()
class GASPROJECT_API UPlayerVitalsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// WBP 안의 같은 이름을 가진 ProgressBar와 자동 연결.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar>HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	

private:
	void RefreshBars();
	void OnVitalsChanged(const FOnAttributeChangeData& Data);
	void UnBindASC();

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

};

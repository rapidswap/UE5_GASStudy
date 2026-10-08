// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/GPUserWidget.h"
#include "GPHpBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API UGPHpBarWidget : public UGPUserWidget
{
	GENERATED_BODY()


public:
	UGPHpBarWidget(const FObjectInitializer& ObjectInitializer);

	// 최대 체력 값 설정 함수.
	FORCEINLINE void SetMaxHp(float NewMaxhp) { MaxHp = NewMaxhp; }


	virtual void UpdateStatBar(float CurrentHp);

public:
	// UMG가 초기화될 때 호출되는 함수
	void NativeConstruct() override;

protected:
	// 게이지를 보여주기 위한 프로그레스 바 참조 변수.
	UPROPERTY()
	TObjectPtr<class UProgressBar> HpProgressBar;

	UPROPERTY()
	float MaxHp;
	
};

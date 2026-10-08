// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GPCharacterWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGPCharacterWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GASPROJECT_API IGPCharacterWidgetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 위젯이 캐릭터에 자신의 정보를 전달할 때 사용할 함수.
	virtual void SetupCharacterWidget(class UGPUserWidget* InUserWidget) = 0;
};

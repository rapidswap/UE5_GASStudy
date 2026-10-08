// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GPWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API UGPWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void InitWidget() override;
	
};

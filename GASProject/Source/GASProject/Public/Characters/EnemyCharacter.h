// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "EnemyCharacter.generated.h"

class UWidgetComponent;
class UVitalsWidget;

/**
 * 
 */
UCLASS()
class GASPROJECT_API AEnemyCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCharacter(/*const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()*/);

public:
	UFUNCTION()
	virtual void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	// Interface

	void OnAttack_Implementation() override;
	void OnHit_Implementation() override;
	void OnDie_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> VitalsWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVitalsWidget> VitalsWidgetClass;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterBase.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API AEnemyCharacter : public APlayerCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	virtual void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	// Interface

	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void OnAttack_Implementation() override;
	void OnHit_Implementation() override;
	void OnDie_Implementation() override;

protected:

};

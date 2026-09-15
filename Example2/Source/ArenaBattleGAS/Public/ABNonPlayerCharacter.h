// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ABBaseCharacter.h"
#include "ABNonPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLEGAS_API AABNonPlayerCharacter : public AABBaseCharacter
{
	GENERATED_BODY()
	
public:
	AABNonPlayerCharacter();


	virtual void PossessedBy(AController* NewController) override;
};

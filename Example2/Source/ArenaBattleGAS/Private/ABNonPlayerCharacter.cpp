// Fill out your copyright notice in the Description page of Project Settings.


#include "ABNonPlayerCharacter.h"
#include "AbilitySystemComponent.h"

AABNonPlayerCharacter::AABNonPlayerCharacter()
{

}

void AABNonPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASC->InitAbilityActorInfo(this, this);

	
}

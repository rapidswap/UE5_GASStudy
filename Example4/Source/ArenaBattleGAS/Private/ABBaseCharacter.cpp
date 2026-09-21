// Fill out your copyright notice in the Description page of Project Settings.


#include "ABBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterAttributeSet.h"


// Sets default values
AABBaseCharacter::AABBaseCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// Attribute Set
	CharacterAttributeSet = CreateDefaultSubobject<UABCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	ASC->AddAttributeSetSubobject<UABCharacterAttributeSet>(CharacterAttributeSet);

	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCapsule"));

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// Weapon
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	// Widget
	HpBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBar"));
	HpBar->SetupAttachment(GetMesh());
	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	HpBar->SetWidgetSpace(EWidgetSpace::Screen);
	HpBar->SetDrawSize(FVector2D(150.0f, 20.f));
	HpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AABBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ABPostInitializeComponents();
	ASC->SetNumericAttributeBase(CharacterAttributeSet->GetMaxHealthAttribute(), 200.0f);
	ASC->SetNumericAttributeBase(CharacterAttributeSet->GetHealthAttribute(), 200.0f);
}


void AABBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	for (const auto& EachAbility : InitialAbilities)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(EachAbility));
	}
}

UAbilitySystemComponent* AABBaseCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}



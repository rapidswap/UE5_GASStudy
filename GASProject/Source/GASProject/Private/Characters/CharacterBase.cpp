// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Tags/GASGameplayTags.h"
#include "UI/Widgets/GPWidgetComponent.h"
#include "UI/Widgets/GPHpbarWidget.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterBase)


// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 컨트롤러에 맞춰서 액터가 회전하지 않도록 설정.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 현재 회전 방향으로 MovementComponent 회전.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// 캐릭터의 이동속도 및 점프 높이.
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// ASC 생성.
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// Attributeset 컴포넌트처럼 등록하기.
	BaseAttributeSet = CreateDefaultSubobject<UBaseAttributeSet>(TEXT("BaseAttributeSet"));

	HpBar = CreateDefaultSubobject<UGPWidgetComponent>(TEXT("Widget"));

	// 위젯 컴포넌트는 씬 컴포넌트이기 때문에 계층 설정.
	HpBar->SetupAttachment(GetMesh());

	// 캐릭터 머리 위에 보일 수 있도록 위치 조정.
	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));

	// 위젯 설정.
	HpBar->SetWidgetSpace(EWidgetSpace::Screen);

	// UI가 그려질 크기 설정.
	HpBar->SetDrawSize(FVector2D(150.0f, 15.0f));

	// 콜리전 끄기.
	HpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// GameplayAbility 등록.
	if (ASC)
	{
		// 컨트롤러가 없는 에너미도 GAS 소유자 정보를 갖도록 공통 초기화.
		ASC->InitAbilityActorInfo(this, this);

		if (!HasAuthority())
		{
			return;
		}

		// 공격 Notify의 GameplayEvent를 받아 실제 충돌 검사를 실행.
		//if (!HitTraceDelegateHandle.IsValid())
		//{
		//	HitTraceDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(HitTraceTag)
		//		.AddUObject(this, &ThisClass::HitTrace);
		//}

		if (BaseAttributeInitEffect)
		{
			// GameplayAbility와 동일하게 원본 형태로 사용하는 것이 아닌 FGameplayEffectSpecHandle구조체로 변환후 ASC 등록 사용.
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle BaseEffectSpecHandle = ASC->MakeOutgoingSpec(BaseAttributeInitEffect, 1, EffectContext);
			if (BaseEffectSpecHandle.IsValid())
			{
				// Handle을 가지고 타겟의 ASC에 Effect 적용. (GA의 Activate 처리).
				// BaseEffectSpecHandle.Data -> TSharedPtr<UGameplayEffectSpec>
				ASC->ApplyGameplayEffectSpecToTarget(*BaseEffectSpecHandle.Data.Get(), ASC);
			}
		}

		AttackAbilityHandles.Reset();
		AttackAbilityHandles.Reserve(AttackAbilities.Num());
		for (const auto& Ability : AttackAbilities)
		{
			if (!Ability)
			{
				continue;
			}

			// 공격 Ability 등록. 현재 Ability는 SubclassOf 형태이므로 인스턴스가 아닌 Class 타입
			// 그렇기 때문에 실제 ClassObject를 가져와서 ASC에 할당해야함. 이 경우 CDO(Class Default Object)를 가져오면 됨
			UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
			// 두 번째 인자는 Ability내부의 GameplayEffect의 레벨 값
			FGameplayAbilitySpec AttackAbilitySpec(AbilityCDO, 1);
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AttackAbilitySpec);
			if (Handle.IsValid())
			{
				AttackAbilityHandles.Emplace(Handle);
			}
		}
	}
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}




void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ASC의 OwnerActor 설정
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void ACharacterBase::OnAttack_Implementation()
{
}

void ACharacterBase::OnHit_Implementation()
{
}

void ACharacterBase::OnDie_Implementation()
{
}




void ACharacterBase::HitTrace(const FGameplayEventData* /*InPlayLoad*/)
{
	const FVector Start = GetActorLocation();

	FVector ForwardVector = GetActorForwardVector();
	ForwardVector.Z = 0.0f;
	ForwardVector.Normalize();

	const FVector End = Start + (ForwardVector * CheckDistance);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		Start,
		End,
		SphereRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true
	);

	for (const FHitResult& Result : HitResults)
	{
		if (AActor* Actor = Result.GetActor())
		{
			if (Actor->GetClass()->ImplementsInterface(UCombatActorInterface::StaticClass()))
			{
				ICombatActorInterface::Execute_OnHit(Actor);
			}
		}
	}

}

void ACharacterBase::SetupCharacterWidget(UGPUserWidget* InUserWidget)
{
	// 델리게이트 등록.
	BaseAttributeSet->OnStatChanged.AddUObject(this, &ThisClass::RefreshStatWidgets);

}

void ACharacterBase::RefreshStatWidgets()
{
	if (!HpBar)
	{
		return;
	}

	UGPHpBarWidget* HpBarWidget = Cast<UGPHpBarWidget>(HpBar->GetWidget());

	if (HpBarWidget)
	{
		HpBarWidget->SetMaxHp(BaseAttributeSet->GetMaxHealth());
		HpBarWidget->UpdateStatBar(BaseAttributeSet->GetHealth());
	}
}



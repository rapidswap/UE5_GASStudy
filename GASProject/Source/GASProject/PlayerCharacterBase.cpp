// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BaseAttributeSet.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerCharacterBase)

UE_DEFINE_GAMEPLAY_TAG(MovingBlockTag, "Character.State.MovementBlocked");
UE_DEFINE_GAMEPLAY_TAG(NowAttackingTag, "Character.State.Attacking");
UE_DEFINE_GAMEPLAY_TAG(NextAttackTag, "Character.Event.NextAttack");
UE_DEFINE_GAMEPLAY_TAG(HitTraceTag, "Character.Event.HitTrace");

// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
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

	// 카메라 붐 생성 및 설정.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); // 액터의 루트 컴포넌트 하위에 등록.
	CameraBoom->TargetArmLength = 500.0f; // 카메라붐의 길이.
	CameraBoom->bUsePawnControlRotation = true; // 카메라붐이 Pawn의 회전에 맞춰서 같이 회전.

	// 카메라 생성.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 카메라는 카메라붐 아래에 등록.
	FollowCamera->bUsePawnControlRotation = false;

	// ASC 생성.
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// Attributeset 컴포넌트처럼 등록하기.
	BaseAttributeSet = CreateDefaultSubobject<UBaseAttributeSet>(TEXT("BaseAttributeSet"));

}

// Called when the game starts or when spawned
void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// GameplayAbility 등록.
	if (ASC)
	{
		// 공격 Notify의 GameplayEvent를 받아 실제 충돌 검사를 실행.
		if (!HitTraceDelegateHandle.IsValid())
		{
			HitTraceDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(HitTraceTag)
				.AddUObject(this, &ThisClass::HitTrace);
		}

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

		AttackAbilityHandles.Reserve(AttackAbilities.Num());
		for (const auto& Ability : AttackAbilities)
		{
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
	

void APlayerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 캐릭터 종료 시 피격 판정 이벤트 구독 해제.
	if (IsValid(ASC) && HitTraceDelegateHandle.IsValid())
	{
		if (auto* HitTraceCallbacks = ASC->GenericGameplayEventCallbacks.Find(HitTraceTag))
		{
			HitTraceCallbacks->Remove(HitTraceDelegateHandle);
		}
	}
	HitTraceDelegateHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	ensure(InputMappingContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA와 실행 동작을 맵핑.
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
	}
	

}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ASC의 OwnerActor 설정
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* APlayerCharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void APlayerCharacterBase::RemoveComboAttackBinding_Implementation()
{
	RemoveAttackDelegate();
}

void APlayerCharacterBase::OnAttack_Implementation()
{
}

void APlayerCharacterBase::OnHit_Implementation()
{
}

void APlayerCharacterBase::OnDie_Implementation()
{
}


void APlayerCharacterBase::Move(const FInputActionValue& InValue)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MovementVector = InValue.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// Get Forward Vector.
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Get Right Vector.
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Add Input.
	AddMovementInput(ForwardDir, MovementVector.Y); // 정면.
	AddMovementInput(RightDir, MovementVector.X); // 좌우.
}

void APlayerCharacterBase::Look(const FInputActionValue& InValue)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D LookAxisVector = InValue.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X); // 좌우 YAW 회전.
	AddControllerPitchInput(LookAxisVector.Y); // 상하 Pitch 회전.
}

void APlayerCharacterBase::Attack()
{

	if (ASC->HasMatchingGameplayTag(NowAttackingTag))
	{
		// 다음 공격 호출.
		bCallNextAttack = true;
	}
	else
	{
		ComboIndex = 0;
		bCallNextAttack = false;

		if (!AttackAbilityHandles.IsValidIndex(ComboIndex))
		{
			return;
		}

		// TryActivateAbility: 호출하면 내부에서 체크한 후에 Ability를 실행.
		const bool bActivate = ASC->TryActivateAbility(AttackAbilityHandles[ComboIndex]);

		// 공격 Ability가 실행되고 다음 공격 Ability가 있다면.
		if(bActivate&& (ComboIndex+1<AttackAbilityHandles.Num()))
		{
			// 특정 GameplayTag가 등록되었을 때 발동하고 싶은 함수를 Delegate에 등록.
			// Handle을 따로 관리하는 이유는 Delegate를 다 사용했으면 삭제하는 과정이 필요하기 때문.
			ComboDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(NextAttackTag).AddUObject(this, &ThisClass::StartNextAttack);
		}
	}
}

void APlayerCharacterBase::Jump()
{
	Super::Jump();
}

void APlayerCharacterBase::StartNextAttack(const FGameplayEventData* InPlayLoad)
{
	if (bCallNextAttack == false)
	{
		return;
	}

	if (ComboIndex + 1 < AttackAbilityHandles.Num())
	{
		ComboIndex++;
		bCallNextAttack = false;
		const bool bActivate = ASC->TryActivateAbility(AttackAbilityHandles[ComboIndex]);
		// 실행하지 못했다면 기존 등록했던 Delegate 구독 해제.
		if (!bActivate)
		{
			RemoveAttackDelegate();
		}
	}
}

void APlayerCharacterBase::RemoveAttackDelegate()
{
	if (ComboDelegateHandle.IsValid())
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(NextAttackTag).Remove(ComboDelegateHandle);
		ComboDelegateHandle.Reset();
	}
}

void APlayerCharacterBase::HitTrace(const FGameplayEventData* /*InPlayLoad*/)
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


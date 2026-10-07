# GASProject 소스 구조

`Public`에는 헤더(`.h`), `Private`에는 구현(`.cpp`)을 둡니다. 두 폴더 안의 역할별 경로는 같습니다.

| 경로 | 역할 | 클래스 / 파일 |
| --- | --- | --- |
| `Characters` | 캐릭터 공통 처리와 플레이어·적 구현 | `CharacterBase`, `PlayerCharacter`, `EnemyCharacter` |
| `GAS/Abilities` | 공격과 스프린트 Gameplay Ability | `PlayMontageAndWait`, `PlayerSprintAbility` |
| `GAS/Attributes` | 체력·마나·스태미나 Attribute Set | `BaseAttributeSet`, `PlayerAttributeSet` |
| `GAS/Tags` | 네이티브 Gameplay Tag 선언과 정의 | `GASGameplayTags` |
| `Animation/Notifies` | 타격 검사와 콤보 애니메이션 Notify | `AnimNotify_HitTrace`, `AnimNotify_ComboCancel`, `AnimNotifyState_CallNextCombo` |
| `Interfaces` | 전투와 콤보 처리 인터페이스 | `CombatActorInterface`, `ComboAttackInterface` |
| `UI/Widgets` | ASC 구독 베이스, 공통 체력·마나와 플레이어 스태미나 표시 위젯 | `AttributeWidgetBase`, `VitalsWidget`, `PlayerVitalsWidget` |

모듈 진입점은 `Public/GASProject.h`, `Private/GASProject.cpp`입니다. 모듈 의존성은 루트의 `GASProject.Build.cs`에서 설정합니다.

## 파일을 참조하는 방법

프로젝트 헤더는 `Public` 아래의 경로로 include합니다.

```cpp
#include "Characters/PlayerCharacter.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/GASGameplayTags.h"
#include "UI/Widgets/PlayerVitalsWidget.h"
```

각 헤더의 `*.generated.h` include는 기존 파일명을 유지하고, 다른 include보다 마지막에 둡니다.

## Blueprint 연결

소스 폴더 이동 후에도 모듈명과 클래스명은 같습니다. Blueprint 부모 클래스와 `/Script/GASProject.*` 참조는 같은 클래스를 가리킵니다.

## 스프린트 연결

- `BP_ThirdPersonCharacter`의 `SprintAbility`에는 `BPGA_Sprint`를 지정합니다.
- `BPGA_Sprint`의 `StaminaDrainEffect`에는 `BPGE_SprintStamina`를 지정합니다. 소모 이펙트를 설정하지 않은 C++ 클래스만 연결하면 스프린트가 활성화되지 않습니다.
- `SprintAction`은 `IA_Sprint`, `InputMappingContext`는 `IMC_Default`입니다. 왼쪽 Shift로 시작하고 키를 놓거나 입력이 취소되면 종료합니다.
- 현재 플레이어의 걷기 속도는 200, `BPGA_Sprint`의 `SprintSpeed`는 600입니다. 속도와 이펙트는 BP 기본값에서 변경할 수 있습니다.

## Vitals 위젯

- `UAttributeWidgetBase`: 표시 대상 ASC 지정, 속성 변경 구독/해제와 막대 비율 계산을 담당합니다.
- `UVitalsWidget`: 베이스를 상속하고 공통 `HealthBar`, `ManaBar`를 표시합니다.
- `UPlayerVitalsWidget`: 공통 위젯을 상속하고 `StaminaBar`와 스태미나 변경 구독을 추가합니다.
- 플레이어와 Enemy는 각각 자기 ASC를 `SetTargetASC()`로 전달합니다.
- `Content/UI/WBP_PlayerVitals`는 `UPlayerVitalsWidget`을 사용하며 체력·스태미나에 마나 막대를 추가했습니다.
- `Content/UI/WBP_Vitals`는 `UVitalsWidget`을 사용하며 체력·마나를 표시합니다.
- 플레이어 HUD는 왼쪽 아래에 표시합니다. `BP_ThirdPersonCharacter`의 `UI > Vitals`에서 `PlayerVitalsViewportSize`(기본 300×72), `PlayerVitalsViewportOffset`(기본 24, -24)을 변경할 수 있습니다.
- HUD 위치·크기·앵커·정렬은 `SetPlayerViewportLayout()`에서 하나의 뷰포트 슬롯으로 설정합니다. UE 5.8의 개별 위치/크기 설정 함수가 앵커를 초기화하는 문제를 피합니다.
- 막대 자체의 Render Transform은 이동 0, 배율 1로 두고 VerticalBox 슬롯으로 배치합니다. 플레이어 HUD의 화면 위치를 공통 Enemy 위젯의 막대에 적용하지 않습니다.
- `BP_EnemyCharacter`의 `VitalsWidgetClass`에는 `WBP_Vitals`를 연결했습니다. `VitalsWidgetComponent`가 머리 위 표시를 담당합니다. 다른 공통 위젯 BP도 에디터에서 지정할 수 있습니다.
- ProgressBar 이름은 `HealthBar`, `ManaBar`, `StaminaBar`입니다. `ManaBar`는 마나를 표시하지 않는 레이아웃을 위해 선택 항목입니다.

## Vitals 연결 검사

`GASProject.UI.Vitals.AttackUpdatesEnemyHealth` 자동 검사는 실제 공격 BP와 `HitTraceTag` 이벤트를 실행하여 적 체력 감소, 적 ASC 연결, 체력 바 비율 변경과 마나 유지 여부를 확인합니다. 검사는 게임 에셋을 저장하거나 레벨을 변경하지 않습니다.

`GASProject.UI.Vitals.PlayerSprintAndViewport`는 왼쪽 Shift 매핑, 스프린트 입력 이벤트 바인딩, 속도 변경, 실제 주기 GE의 스태미나 소모, 스태미나 바 갱신, 키 해제/취소와 스태미나 고갈 시 종료, HUD의 왼쪽 아래 슬롯 설정을 확인합니다. 화면 없는 검사이므로 실제 키보드 입력이나 화면 렌더링을 확인하는 검사는 아닙니다.

## 빌드

프로젝트 루트에서 PowerShell로 실행합니다. 설치된 UE 5.8 경로를 사용하는 명령입니다.

```powershell
& 'C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat' GASProjectEditor Win64 Development '-Project=C:/Workspace/khj/Wanted5th/GAS_Study/GASProject/GASProject.uproject' -WaitMutex
```

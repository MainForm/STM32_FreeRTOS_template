# STM32 FreeRTOS Template

STM32CubeMX가 생성하는 코드와 사용자가 작성하는 애플리케이션 코드를 분리한 FreeRTOS 프로젝트 템플릿입니다.

각 Task의 실행 함수를 `App` 디렉터리의 개별 C++ 파일에서 관리하도록 구성하여 유지보수성을 높이고, 여러 개발자가 Git으로 협업할 때 동일한 생성 파일을 수정하면서 발생하는 충돌을 줄이는 것을 목표로 합니다.

## 핵심 설계

### CubeMX 생성 코드와 애플리케이션 코드 분리

- `Core`, `Drivers`, `Middlewares`, `cmake`: STM32CubeMX에서 생성하거나 관리하는 코드
- `App`: 사용자가 직접 작성하고 유지보수하는 애플리케이션 코드

Task 생성과 스케줄러 설정은 CubeMX가 생성한 `Core/Src/freertos.c`에서 담당하고, 각 Task의 실제 동작은 `App/Src` 아래의 별도 파일에서 구현합니다.

```text
STM32_FreeRTOS_template/
├── App/
│   └── Src/
│       ├── default_task.cpp
│       └── my_task2.cpp
├── Core/
│   └── Src/
│       ├── freertos.c
│       └── main.c
├── Drivers/
├── Middlewares/
├── cmake/
├── CMakeLists.txt
└── STM32_FreeRTOS_template.ioc
```

이 구조를 사용하면 CubeMX에서 코드를 다시 생성하더라도 애플리케이션 로직을 별도의 파일에 유지할 수 있으며, 생성 코드와 사용자 코드의 책임 범위도 명확해집니다.

### Task Handler 분리

먼저 STM32CubeMX에서 다음과 같이 FreeRTOS와 Task를 설정합니다.

1. **Pinout & Configuration > Middleware and Software Packs > FREERTOS**로 이동합니다.
2. **Interface**를 `CMSIS_V2`로 선택합니다.
3. **Tasks and Queues** 탭에서 Task를 추가하거나 기존 Task를 편집합니다.
4. 각 Task의 이름, 우선순위, Stack 크기, Entry Function을 지정합니다.
5. **Code Generation Option**에서 Handler 생성 방식을 `As weak` 또는 `As external`로 선택합니다.

현재 프로젝트의 `.ioc`에는 다음과 같이 설정되어 있습니다.

| Task | Priority | Stack Size | Entry Function | Code Generation Option |
| --- | --- | ---: | --- | --- |
| `defaultTask` | Normal | 128 Words | `StartDefaultTask` | `As weak` |
| `myTask02` | Low | 128 Words | `StartTask02` | `As external` |

- `As weak`: CubeMX가 `__weak` 기본 구현을 생성합니다. `App/Src`에서 같은 이름의 강한 함수를 정의하면 애플리케이션 구현이 대신 사용됩니다.
- `As external`: CubeMX는 Handler를 `extern`으로 선언하고 구현은 생성하지 않습니다. 따라서 `App/Src`에 반드시 해당 함수를 정의해야 합니다.

CubeMX에서 생성된 `freertos.c`에는 Task Handler의 선언, Task 생성 코드, 필요한 경우 기본 `__weak` 구현만 유지하고 실제 애플리케이션 동작은 `App/Src`로 분리합니다.

```c
void StartDefaultTask(void *argument);
extern void StartTask02(void *argument);

defaultTaskHandle = osThreadNew(
    StartDefaultTask,
    NULL,
    &defaultTask_attributes
);

myTask02Handle = osThreadNew(
    StartTask02,
    NULL,
    &myTask02_attributes
);
```

`StartDefaultTask`의 기본 구현은 `__weak` 함수로 선언되어 있으므로, `App/Src/default_task.cpp`에 같은 이름의 함수를 정의하여 대체할 수 있습니다.

```c
__weak void StartDefaultTask(void *argument)
{
    for (;;)
    {
        osDelay(1);
    }
}
```

`StartTask02`는 `extern`으로 선언하고 실제 구현은 `App/Src/my_task2.cpp`에 둡니다. C 파일에서 호출되는 함수를 C++ 파일에 구현할 때는 이름 맹글링을 방지하기 위해 `extern "C"`를 사용합니다.

```cpp
extern "C"
void StartTask02(void *argument)
{
    while (1)
    {
        // Task 동작 구현
        osDelay(1000);
    }
}
```

## 협업 시 장점

각 Task를 독립적인 파일에서 관리하므로 개발자별 작업 범위를 분리하기 쉽습니다.

- CubeMX 생성 파일에 애플리케이션 로직이 집중되는 것을 방지
- Task별 코드 탐색, 수정 및 테스트 용이
- 서로 다른 Task를 동시에 개발할 때 Git 충돌 가능성 감소
- CubeMX 코드 재생성 시 사용자 로직이 영향을 받을 가능성 감소
- C 기반 생성 코드와 C++ 애플리케이션 코드를 명확하게 연결

예를 들어 한 개발자는 `default_task.cpp`, 다른 개발자는 `my_task2.cpp`를 수정할 수 있으므로 동일한 `freertos.c`를 반복해서 편집하는 상황을 최소화할 수 있습니다.

## 새로운 Task 추가 방법

1. STM32CubeMX에서 Task를 생성하고 우선순위와 Stack 크기를 설정합니다.
2. 생성된 `freertos.c`에서 Task Handler가 `extern` 또는 재정의 가능한 `__weak` 함수로 선언되었는지 확인합니다.
3. `App/Src`에 해당 Task를 구현할 `.cpp` 파일을 생성합니다.
4. Task Handler를 `extern "C"`로 정의합니다.
5. 생성한 파일을 `CMakeLists.txt`의 `target_sources`에 추가합니다.

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    App/Src/default_task.cpp
    App/Src/my_task2.cpp
)
```

> CubeMX에서 코드를 다시 생성한 뒤에는 `freertos.c`의 Handler 선언과 `CMakeLists.txt`의 애플리케이션 소스 등록이 유지되었는지 확인해야 합니다.

## 현재 예제

- `defaultTask`: 1초마다 UART2로 실행 메시지 전송
- `myTask02`: 2초마다 UART2로 실행 메시지 전송
- RTOS API: CMSIS-RTOS2
- 애플리케이션 언어: C++

이 프로젝트는 Task별 소스 분리를 바탕으로 기능을 확장할 수 있는 기본적인 STM32 FreeRTOS 개발 환경을 제공합니다.

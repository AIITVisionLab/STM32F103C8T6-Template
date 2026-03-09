# STM32F103C8T6 FreeRTOS Template

这是一个面向 `STM32F103C8T6` 的 FreeRTOS 模板工程，基于 `HAL + CMSIS + startup + linker script + CMake + Ninja + arm-none-eabi-gcc + OpenOCD + VS Code` 组织。

当前工程已经完成 `FreeRTOS v9.0.0` 的基础移植，默认运行方式不再是裸机空循环，而是：

- `HAL_Init()`
- `SystemClock_Config()`
- 创建默认任务
- `vTaskStartScheduler()`

系统时钟固定为 `HSE 8 MHz -> PLL x9 -> SYSCLK 72 MHz`。HAL 的 `uwTick` 由 `TIM2` 提供，`SysTick/SVC/PendSV` 完全交给 FreeRTOS 内核。

## 当前模板特性

- 芯片：`STM32F103C8T6`
- 内核：`Cortex-M3`
- RTOS：`FreeRTOS v9.0.0`
- 系统主频：`72 MHz`
- 外部高速晶振：`8 MHz`
- HAL 时基：`TIM2`
- FreeRTOS tick：`1 kHz`
- FreeRTOS 堆实现：`heap_4`
- FreeRTOS 动态堆大小：`8 KB`
- 默认任务：每 `500 ms` 递增一次 `g_default_task_heartbeat`

## 工程架构设计

### 目录职责

| 路径 | 作用 |
| --- | --- |
| `.vscode/` | VS Code 工作区配置，封装 `CMake Configure`、`CMake Build`、`Flash`、`Clean` |
| `project/` | 构建系统与目标平台配置，包括 `CMakeLists.txt`、GNU Arm 工具链文件、链接脚本 |
| `user/` | 用户层代码入口，包含 `main.c`、中断处理、HAL 配置、HAL 时基、FreeRTOS 配置与钩子 |
| `Libraries/cmsis/` | Arm CMSIS 内核头文件 |
| `Libraries/stm32f1/` | STM32F1 设备头文件与系统初始化代码 |
| `Libraries/hal/` | STM32F1 HAL/LL 驱动源码与头文件 |
| `Libraries/startup/` | 启动汇编文件与中断向量表 |
| `Libraries/freertos/` | 项目内保留的最小 FreeRTOS 内核子集 |
| `build/` | 构建输出目录，保存 `ELF/HEX/BIN/MAP` 和 `compile_commands.json` |
| `FreeRTOSv9.0.0/` | 迁移时保留的官方源码包；当前构建已不依赖它，可在确认无误后手动删除 |

### 分层关系

```text
user/*.c
  |-- main.c                      应用入口、72 MHz 时钟初始化、默认任务创建
  |-- stm32f1xx_it.c              Cortex-M 异常入口（FreeRTOS 接管 SysTick/SVC/PendSV）
  |-- stm32f1xx_hal_msp.c         HAL 底层外设初始化入口
  |-- stm32f1xx_hal_timebase_tim.c TIM2 HAL 1 ms 时基 + RTOS 友好 HAL_Delay()
  |-- FreeRTOSConfig.h            FreeRTOS 内核配置
  |-- freertos_hooks.c            assert/malloc failed/stack overflow 钩子
  |-- runtime_stubs.c             最小 C 运行时桩（_init/_fini）
  v
Libraries/freertos/Source
  |-- list.c queue.c tasks.c timers.c
  |-- portable/GCC/ARM_CM3/port.c
  |-- portable/MemMang/heap_4.c
  v
Libraries/hal + Libraries/stm32f1 + Libraries/cmsis
  v
startup_stm32f103xb.s + STM32F103XB_FLASH.ld
  v
CMake + Ninja + arm-none-eabi-gcc
  v
template.elf
  |-- objcopy --> template.hex
  |-- objcopy --> template.bin
  |-- linker  --> template.map
```

## FreeRTOS 设计说明

### 中断和时基分工

- `SysTick`：只用于 FreeRTOS 内核 tick
- `SVC` / `PendSV`：由 FreeRTOS 端口层接管
- `TIM2`：只用于 HAL `uwTick`
- `HAL_Delay()`：
  - 调度器未启动或当前处于 ISR 时，使用 HAL tick 忙等
  - 调度器已运行且在线程上下文中时，使用 `vTaskDelay()`

### 任务与调试可见量

模板默认创建一个任务：

- 任务名：`default`
- 优先级：`tskIDLE_PRIORITY + 1`
- 栈深：`256` words
- 行为：每 `500 ms` 递增一次 `g_default_task_heartbeat`

如果你用调试器连接目标板，最直接的 RTOS 启动验收方式是观察以下变量：

- `g_default_task_heartbeat`
- `uwTick`
- `SystemCoreClock`
- `g_freertos_assert_file`
- `g_freertos_assert_line`
- `g_freertos_malloc_failed`
- `g_freertos_stack_overflow_task`

### FreeRTOS 当前配置摘要

当前模板固定使用如下核心配置：

- `configCPU_CLOCK_HZ = SystemCoreClock`
- `configTICK_RATE_HZ = 1000`
- `configMAX_PRIORITIES = 5`
- `configUSE_PREEMPTION = 1`
- `configUSE_MUTEXES = 1`
- `configUSE_RECURSIVE_MUTEXES = 1`
- `configUSE_COUNTING_SEMAPHORES = 1`
- `configUSE_TIMERS = 1`
- `configCHECK_FOR_STACK_OVERFLOW = 2`
- `configUSE_MALLOC_FAILED_HOOK = 1`
- `configTOTAL_HEAP_SIZE = 8 * 1024`
- `configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 15`
- `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`

如果你后续要扩展任务数量、队列、信号量或软件定时器，优先检查：

- `user/FreeRTOSConfig.h`
- `project/STM32F103XB_FLASH.ld`
- `build/template.map`

## 构建设计要点

当前工程的核心构建设计如下：

- 目标内核为 `cortex-m3`，编译参数使用 `-mcpu=cortex-m3 -mthumb`
- 预处理宏定义为 `USE_HAL_DRIVER` 与 `STM32F103xB`
- 启动文件为 `Libraries/startup/startup_stm32f103xb.s`
- 系统文件为 `Libraries/stm32f1/system_stm32f1xx.c`
- 链接脚本为 `project/STM32F103XB_FLASH.ld`，默认内存布局为 `64 KB Flash + 20 KB RAM`
- FreeRTOS 源文件由 `project/CMakeLists.txt` 显式列出，不依赖根目录官方包
- 目标文件名固定为 `template.elf`
- 构建完成后自动生成 `template.hex`、`template.bin`，并执行 `arm-none-eabi-size`

## 环境准备

### 必需工具

请确保以下工具已安装，并且可在命令行中直接访问：

- `arm-none-eabi-gcc`
- `cmake`
- `ninja`
- `openocd`

如果你的 GNU Arm 工具链没有加入系统 `PATH`，可以通过 `project/arm-gnu-toolchain.cmake` 中使用的 `ARM_GNU_TOOLCHAIN_HINT_DIRS` 变量提供查找路径。

### VS Code 建议扩展

如果你使用 VS Code，建议安装以下扩展：

- `clangd`
- `cortex-debug`
- `CMake Tools`

### 调试器前提

当前调试配置默认面向：

- 调试探针：`CMSIS-DAP`
- 调试接口：`SWD`
- OpenOCD 目标脚本：`target/stm32f1x.cfg`

## 使用教程

### 使用 VS Code

1. 用 VS Code 打开本工程根目录。
2. 运行任务 `CMake Configure`，生成 `build/` 目录和 Ninja 构建文件。
3. 运行任务 `CMake Build`，编译生成 `build/template.elf`、`build/template.hex`、`build/template.bin`。
4. 连接开发板与 CMSIS-DAP 调试器后，运行任务 `Flash` 完成下载。
5. 如果需要清理构建结果，运行任务 `Clean`。

### 使用命令行

#### 1. 生成构建目录

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=project/arm-gnu-toolchain.cmake \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
  -GNinja \
  -S project \
  -B build
```

#### 2. 编译工程

```bash
cmake --build build --target all
```

#### 3. 烧录程序

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg -c "program build/template.elf verify reset exit"
```

#### 4. 清理构建目录

```bash
rm -rf build .cache
```

## 输出产物说明

构建完成后，`build/` 目录中主要会出现以下文件：

- `build/template.elf`：调试和烧录的主目标文件
- `build/template.hex`：Intel HEX 格式固件
- `build/template.bin`：裸二进制固件
- `build/template.map`：链接映射文件，可用于分析代码与内存占用
- `build/compile_commands.json`：供 `clangd` 等工具进行代码索引与静态分析

## 二次开发入口

在继续扩展这个模板工程时，优先关注以下入口：

- `user/main.c`
  - 应用入口
  - 默认时钟配置为 `HSE 8 MHz -> 72 MHz`
  - 默认会创建任务并启动调度器，而不是停在空循环
- `user/FreeRTOSConfig.h`
  - FreeRTOS 内核功能、优先级、堆大小和中断优先级配置入口
- `user/stm32f1xx_hal_timebase_tim.c`
  - TIM2 HAL 时基与 `HAL_Delay()` 行为入口
- `user/freertos_hooks.c`
  - FreeRTOS 运行期故障钩子入口
- `user/stm32f1xx_it.c`
  - Cortex-M 异常与外设中断入口
- `user/stm32f1xx_hal_msp.c`
  - HAL 底层初始化入口
- `project/STM32F103XB_FLASH.ld`
  - Flash/RAM 布局定义
- `project/CMakeLists.txt`
  - 构建目标、包含路径、芯片宏、FreeRTOS 源文件参与范围

如果你后续要把模板迁移到其他芯片，至少需要同步检查以下内容：

- 芯片宏定义
- 启动文件
- 链接脚本
- OpenOCD 目标脚本
- 设备头文件与系统文件
- `SystemClock_Config()`
- `FreeRTOSConfig.h` 中的优先级位数和中断优先级

如果你后续要精简模板体积，建议优先做两件事：

- 在 `user/stm32f1xx_hal_conf.h` 中关闭不用的 HAL 模块
- 在 `project/CMakeLists.txt` 中同步收缩参与编译的驱动源文件范围

## 推荐使用方式

这个模板比较适合作为以下场景的起点：

- 新建 `STM32F103C8T6` FreeRTOS 项目
- 搭建自己的任务/驱动/BSP 分层工程
- 作为 VS Code + CMake + OpenOCD 的轻量化 RTOS 开发基线
- 作为后续串口、DMA、GPIO、中断、软件定时器和同步原语扩展的底座

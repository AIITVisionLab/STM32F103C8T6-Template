# STM32F103C8T6 Template

这是一个面向 `STM32F103C8T6` 的裸机模板工程，基于 `HAL + CMSIS + startup + linker script + CMake + Ninja + arm-none-eabi-gcc + OpenOCD + VS Code` 组织。

工程默认提供最小可运行框架：`main()` 完成 `HAL_Init()` 与 `SystemClock_Config()` 后进入空循环，系统时钟默认配置为 `64 MHz`。你可以在此基础上继续添加 GPIO、串口、定时器、中断和外设驱动代码，而不需要从零重新搭建启动、链接、编译和烧录环境。

## 工程架构设计

### 目录职责

| 路径 | 作用 |
| --- | --- |
| `.vscode/` | VS Code 工作区配置，封装 `CMake Configure`、`CMake Build`、`Flash`、`Clean` 与 `Debug with OpenOCD` |
| `project/` | 构建系统与目标平台配置，包括 `CMakeLists.txt`、GNU Arm 工具链文件、链接脚本 |
| `user/` | 用户层代码入口，包含 `main.c`、中断处理、HAL 配置、MSP 初始化和最小运行时桩 |
| `Libraries/cmsis/` | Arm CMSIS 内核头文件 |
| `Libraries/stm32f1/` | STM32F1 设备头文件与系统初始化代码 |
| `Libraries/hal/` | STM32F1 HAL/LL 驱动源码与头文件 |
| `Libraries/startup/` | 启动汇编文件与中断向量表 |
| `build/` | 构建输出目录，保存 `ELF/HEX/BIN/MAP` 和 `compile_commands.json` |

### 分层关系

```text
user/*.c
  |-- main.c                  应用入口、时钟初始化
  |-- stm32f1xx_it.c          中断服务函数
  |-- stm32f1xx_hal_msp.c     HAL 底层外设初始化入口
  |-- stm32f1xx_hal_conf.h    HAL 模块开关与系统参数
  |-- runtime_stubs.c         最小 C 运行时桩（_init/_fini）
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

### 构建设计要点

当前工程的核心构建设计如下：

- 目标内核为 `cortex-m3`，编译参数使用 `-mcpu=cortex-m3 -mthumb`
- 预处理宏定义为 `USE_HAL_DRIVER` 与 `STM32F103xB`
- 启动文件为 `Libraries/startup/startup_stm32f103xb.s`
- 系统文件为 `Libraries/stm32f1/system_stm32f1xx.c`
- 链接脚本为 `project/STM32F103XB_FLASH.ld`，默认内存布局为 `64 KB Flash + 20 KB RAM`
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
5. 如果需要在线调试，启动 `Debug with OpenOCD` 配置。
6. 如果需要清理构建结果，运行任务 `Clean`。

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
  - 应用主入口
  - 默认系统时钟初始化位于 `SystemClock_Config()`
  - 当前默认使用 `HSI / 2 * PLL x16 = 64 MHz`
- `user/stm32f1xx_it.c`
  - Cortex-M 异常与外设中断服务函数入口
- `user/stm32f1xx_hal_msp.c`
  - HAL 外设底层初始化入口，外设 GPIO、时钟、DMA、中断优先级通常从这里展开
- `user/stm32f1xx_hal_conf.h`
  - HAL 模块开关、时钟常量、断言和系统参数配置入口
- `project/STM32F103XB_FLASH.ld`
  - Flash/RAM 布局定义
- `project/CMakeLists.txt`
  - 构建目标、包含路径、芯片宏、源文件参与范围

如果你要把模板迁移到其他芯片，至少需要同步检查以下内容：

- 芯片宏定义
- 启动文件
- 链接脚本
- OpenOCD 目标脚本
- 设备头文件与系统文件

如果你后续要精简模板体积，建议优先做两件事：

- 在 `user/stm32f1xx_hal_conf.h` 中关闭不用的 HAL 模块
- 在 `project/CMakeLists.txt` 中同步收缩参与编译的驱动源文件范围

## 推荐使用方式

这个模板比较适合作为以下场景的起点：

- 新建 `STM32F103C8T6` 裸机项目
- 搭建自己的通用外设驱动仓库
- 作为 VS Code + CMake + OpenOCD 的轻量化开发基线
- 作为后续 FreeRTOS、驱动分层或 BSP 抽象的底座

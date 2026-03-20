# STM32F103C8T6 Template

这是一个面向 `STM32F103C8T6` 的裸机分层骨架工程，基于 `HAL + CMSIS + startup + linker script + CMake + Ninja + arm-none-eabi-gcc + OpenOCD + VS Code` 组织。

当前版本的重点不再是“把所有代码都放进 `user/`”，而是把模板整理成可长期演进的分层底座：

- `Libraries/` 和 `project/` 继续作为稳定的 vendor/build 基础设施
- `platform/` 负责 HAL 收口、时钟、Tick、中断桥接和临界区
- `board/custom/` 负责板级占位
- `drivers/`、`services/`、`app/` 负责上层骨架入口
- `user/` 只保留厂家兼容钩子，不再承载业务实现

## 目录职责

| 路径            | 作用                                                 |
| --------------- | ---------------------------------------------------- |
| `.vscode/`      | VS Code 工作区配置、构建/烧录/调试工作流             |
| `project/`      | CMake、工具链、链接脚本、调试目标描述                |
| `Libraries/`    | 启动文件、CMSIS、设备头文件、HAL 源码                |
| `config/`       | `target`、`board`、`product` 编译期配置聚合          |
| `common/`       | 通用状态码与断言能力                                 |
| `platform/`     | 时钟初始化、Tick、IRQ 桥接、MSP 收口、临界区         |
| `board/custom/` | 自定义板卡占位层，后续补真实资源表                   |
| `drivers/`      | 驱动层统一入口骨架                                   |
| `services/`     | 服务层统一入口骨架                                   |
| `app/`          | 应用层统一入口骨架                                   |
| `user/`         | `main.c`、中断桥接、HAL MSP 桥接、HAL 配置、运行时桩 |
| `docs/`         | 架构说明与设计文档                                   |

## 启动与调度链

当前默认启动链如下：

```text
Reset_Handler
  -> SystemInit()
  -> __libc_init_array()
  -> main()
     -> platform_init()
     -> board_init()
     -> drivers_init()
     -> services_init()
     -> app_init()
     -> while (1)
        {
          platform_poll();
          drivers_poll();
          services_poll();
          app_poll();
        }
```

其中：

- `HAL_Init()` 和系统时钟切换已收口到 `platform_init()`
- `user/stm32f1xx_it.c` 只负责把异常和 `SysTick` 转发到 `platform_irq_*`
- `user/stm32f1xx_hal_msp.c` 只负责把 HAL 的 MSP 钩子转发到 `platform_msp_*`
- 默认时钟策略保持不变，仍是 `HSI / 2 * PLL x16 = 64 MHz`

## 依赖规则

推荐长期保持以下单向依赖：

```text
app -> services -> drivers -> board/platform -> HAL/CMSIS
common -> 可被多层复用
```

当前骨架已按这个方向收口：

- `app/`、`services/`、`drivers/` 的公共头文件不直接包含 `stm32f1xx_hal.h`
- `main.h` 已退役，避免用“总入口头文件”向上层泄漏 HAL
- HAL 模块只保留当前骨架真正需要的最小集合

## 构建设计

`project/CMakeLists.txt` 现在改为显式维护源码列表，不再使用宽泛的 `file(GLOB ...)` 收集项目代码。

当前显式纳入构建的关键部分包括：

- Vendor：`stm32f1xx_hal.c`、`stm32f1xx_hal_cortex.c`、`stm32f1xx_hal_flash.c`、`stm32f1xx_hal_gpio.c`、`stm32f1xx_hal_rcc.c`、`stm32f1xx_hal_rcc_ex.c`
- Platform：`platform.c`、`platform_irq.c`、`platform_msp.c`
- Skeleton：`board.c`、`drivers.c`、`services.c`、`app.c`
- User bridge：`main.c`、`stm32f1xx_it.c`、`stm32f1xx_hal_msp.c`、`runtime_stubs.c`

这样做的好处是：

- 构建输入更清晰
- HAL 裁剪与代码结构同步
- 后续新增模块时能明确知道自己属于哪一层

## 环境准备

请确保以下工具可直接从命令行访问：

- `arm-none-eabi-gcc`
- `cmake`
- `ninja`
- `openocd`

如果使用 VS Code，建议安装：

- `clangd`
- `cortex-debug`
- `CMake Tools`

## 使用方法

### 1. 配置工程

建议优先使用一个全新的构建目录，避免历史缓存干扰：

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=project/arm-gnu-toolchain.cmake \
  -GNinja \
  -S project \
  -B build-clean
```

### 2. 编译工程

```bash
cmake --build build-clean
```

### 3. 烧录程序

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg -c "program build-clean/template.elf verify reset exit"
```

## 产物说明

构建完成后会生成：

- `template.elf`
- `template.hex`
- `template.bin`
- `template.map`
- `compile_commands.json`

## 二次开发建议

后续继续扩展时，建议按以下方式演进：

- 在 `board/custom/` 中补齐真实板卡资源定义
- 在 `drivers/` 中新增具体外设驱动模块，而不是直接改 `user/`
- 在 `services/` 中承接日志、事件、协议、软件定时器等通用机制
- 在 `app/` 中只保留产品行为、状态机和业务编排
- 在 `user/stm32f1xx_hal_conf.h` 与 `project/CMakeLists.txt` 中同步收紧 HAL 模块范围

当前仓库已经是一个可编译的分层骨架，但仍然故意保持“功能留白”，方便你按具体产品继续往上长。

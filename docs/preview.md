# STM32F103C8T6 模板工程架构预览

## 项目定位

这个仓库现在是一个已经完成“分层骨架重构”的 STM32F103C8T6 裸机模板，而不是早期那种所有用户代码都堆在 `user/` 的模板基线。

它的目标不是直接提供某个产品功能，而是提供一套更适合长期演进的工程骨架：

- 启动文件、中断向量表、CMSIS、HAL、链接脚本、CMake 保持为稳定底座
- `platform/` 负责芯片级收口
- `board/custom/` 负责板级占位
- `drivers/`、`services/`、`app/` 负责上层可扩展入口
- `user/` 收缩为厂家兼容桥接层

当前版本仍然只保留空骨架，不包含 LED、串口、按键、日志、事件队列等产品模块。

## 软件分层与目录职责

当前工程可以理解为下面这组层次：

```text
.vscode/      开发工作流封装层
project/      构建、链接、工具链配置层
Libraries/    vendor 基础设施层
config/       编译期配置层
common/       通用基础能力层
platform/     芯片适配层
board/custom/ 板级支持层
drivers/      驱动层
services/     服务层
app/          应用层
user/         厂家兼容桥接层
```

### 1. `Libraries/`：稳定底座

`Libraries/` 仍然负责：

- 启动文件与中断向量表
- CMSIS 内核头文件
- STM32F1 设备头文件与系统文件
- STM32 HAL 源码和头文件

这层不承载项目逻辑，只作为底层依赖存在。

### 2. `project/`：构建系统

`project/` 负责把各层源码组装成最终固件，核心文件包括：

- `CMakeLists.txt`
- `arm-gnu-toolchain.cmake`
- `STM32F103XB_FLASH.ld`
- `STM32F103xx.svd`

和旧版本不同的是，当前 `CMakeLists.txt` 已经改为显式列出源码分组，不再用宽泛的 `file(GLOB ...)` 收集项目代码。

### 3. `config/`：编译期配置聚合

这一层把配置拆成三类：

- `config/target/target_config.h`
- `config/board/board_config.h`
- `config/product/product_config.h`

并通过 `config/project_config.h` 统一聚合。

当前默认配置为：

- 目标芯片：`STM32F103C8T6`
- 板级占位：`custom`
- 产品占位：`template`
- 默认时钟目标：`64 MHz`

### 4. `common/`：通用基础能力

当前 `common/` 先放两类最小通用能力：

- `project_status_t`
- `PROJECT_ASSERT()` / `project_assert_failed()`

这样后续各层都能共用基础状态码和断言，而不需要依赖具体硬件层。

### 5. `platform/`：芯片适配层

这是本次重构最关键的一层之一。当前它负责：

- `platform_init()` 中收口 `HAL_Init()` 和系统时钟配置
- `platform_poll()` 空轮询入口
- `platform_get_tick_ms()` 统一取 Tick
- `platform_enter_critical()` / `platform_exit_critical()` 临界区接口
- `platform_panic()` 统一致命故障停机入口
- `platform_irq_*` 中断桥接目标
- `platform_msp_*` HAL MSP 收口点

当前默认系统时钟策略仍然保持原模板设置：

```text
HSI 8 MHz
  -> /2
  -> PLL x16
  -> SYSCLK 64 MHz
```

也就是说，这次重构改变的是“时钟配置的位置和职责”，不是“时钟参数本身”。

### 6. `board/custom/`：板级占位层

当前目标是“自定义板卡，但资源尚未定义”，所以这一层现在只保留：

- `board_init()`
- 自定义板卡占位头文件

它还没有引入 LED、按键、电源脚、传感器通道或串口资源表。后续一旦板卡资源明确，就应该优先在这一层补齐，而不是直接把引脚号写进应用层。

### 7. `drivers/`、`services/`、`app/`：上层骨架

本轮只建立统一入口：

- `drivers_init()` / `drivers_poll()`
- `services_init()` / `services_poll()`
- `app_init()` / `app_poll()`

这些模块当前都还是空实现，但它们已经把后续扩展的分工边界先固定住了。

### 8. `user/`：厂家兼容桥接层

当前 `user/` 只保留以下职责：

| 文件 | 作用 |
| --- | --- |
| `main.c` | 统一启动链与主循环调度入口 |
| `stm32f1xx_it.c` | Cortex-M 异常和 `SysTick` 到 `platform_irq_*` 的桥接 |
| `stm32f1xx_hal_msp.c` | HAL MSP 到 `platform_msp_*` 的桥接 |
| `stm32f1xx_hal_conf.h` | HAL 模块与系统参数配置 |
| `runtime_stubs.c` | `_init()` / `_fini()` 运行时桩 |

变化最大的点有两个：

- `main.h` 已退役，不再用一个公共头把 HAL 暴露给整个工程
- `stm32f1xx_it.h` 也不再作为上层共享入口存在

这意味着 `user/` 现在更像“与厂家模板接口兼容的薄适配层”，而不是“业务开发主目录”。

## 启动与运行流程

### 1. 从复位到 `main()`

启动主链保持 Cortex-M 经典流程：

```text
Reset_Handler
  -> SystemInit()
  -> .data 搬运
  -> .bss 清零
  -> __libc_init_array()
  -> main()
```

这部分仍由：

- `Libraries/startup/startup_stm32f103xb.s`
- `project/STM32F103XB_FLASH.ld`
- `Libraries/stm32f1/system_stm32f1xx.c`

共同完成。

### 2. `main()` 之后的统一初始化链

当前 `main()` 已被重写为统一分阶段入口：

```text
main()
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

每个 `*_init()` 都统一返回 `project_status_t`。只要任一阶段失败，`main()` 就会调用 `platform_panic(status)` 停机。

这让初始化顺序固定下来，也让以后新增模块时知道该挂在哪一层。

### 3. `platform_init()` 的职责

在当前版本里，`platform_init()` 是芯片层初始化的唯一公共入口。它内部完成：

- `HAL_Init()`
- 默认 64 MHz 时钟切换

因此，旧模板里直接暴露在 `main.c` 的 `SystemClock_Config()` 已经消失，不再成为应用入口的一部分。

## 中断与异常处理模型

### 1. `user/stm32f1xx_it.c` 的新角色

当前 `stm32f1xx_it.c` 不再自己实现复杂逻辑，而是只做桥接：

- `SysTick_Handler()` -> `platform_irq_systick()`
- `NMI_Handler()` -> `platform_irq_fault(PLATFORM_FAULT_NMI)`
- `HardFault_Handler()` -> `platform_irq_fault(PLATFORM_FAULT_HARD)`
- `MemManage_Handler()` -> `platform_irq_fault(PLATFORM_FAULT_MEMMANAGE)`
- `BusFault_Handler()` -> `platform_irq_fault(PLATFORM_FAULT_BUS)`
- `UsageFault_Handler()` -> `platform_irq_fault(PLATFORM_FAULT_USAGE)`

这让 `user/` 不再成为故障处理逻辑的实际归属地。

### 2. `platform_irq_*` 的职责

当前 `platform_irq_*` 负责两件事：

- 在 `platform_irq_systick()` 中调用 `HAL_IncTick()`
- 在 `platform_irq_fault()` 中统一转入 `platform_panic()`

现在它还很薄，但已经把“向量入口”和“平台故障处理”分离开了。

### 3. MSP 收口

`HAL_MspInit()` 与 `HAL_MspDeInit()` 现在只在 `user/stm32f1xx_hal_msp.c` 中做一次转发：

```text
HAL_MspInit()   -> platform_msp_init()
HAL_MspDeInit() -> platform_msp_deinit()
```

这样后续 GPIO、DMA、UART、TIM 等 MSP 细节都可以继续下沉到 `platform/`，不会反向污染应用层。

## 构建架构与 HAL 裁剪

### 1. 源码收集方式已经收紧

当前项目源码按层显式列出，例如：

- `VENDOR_SOURCES`
- `COMMON_SOURCES`
- `PLATFORM_SOURCES`
- `BOARD_SOURCES`
- `DRIVER_SOURCES`
- `SERVICE_SOURCES`
- `APP_SOURCES`
- `USER_SOURCES`

这样可以明确回答“某个源码为什么被编进来”，也方便后续按层继续拆子模块。

### 2. HAL 模块已经从“全量模板”收缩为“当前最小集合”

当前 `user/stm32f1xx_hal_conf.h` 只启用了这几个 HAL 模块：

- `HAL_CORTEX_MODULE_ENABLED`
- `HAL_FLASH_MODULE_ENABLED`
- `HAL_GPIO_MODULE_ENABLED`
- `HAL_RCC_MODULE_ENABLED`

与之对应，当前构建也只保留当前骨架真正用到的 HAL 源文件：

- `stm32f1xx_hal.c`
- `stm32f1xx_hal_cortex.c`
- `stm32f1xx_hal_flash.c`
- `stm32f1xx_hal_gpio.c`
- `stm32f1xx_hal_rcc.c`
- `stm32f1xx_hal_rcc_ex.c`

这意味着：

- HAL 配置和参与编译的 HAL 源文件是一致的
- 未来启用 GPIO/UART/SPI/TIM 等模块时，必须同时扩展 HAL 配置和 CMake 源列表

### 3. 为什么这一步重要

在模板阶段，“全量 HAL + 链接裁剪”虽然省事，但会弱化工程边界。现在把 HAL 显式收紧后：

- 上层更难误用未准备好的外设接口
- 构建系统更接近真实产品约束
- 架构分层和构建输入开始一致

## 依赖规则

当前骨架遵守的目标依赖关系是：

```text
app -> services -> drivers -> board/platform -> HAL/CMSIS
common -> 可被多层复用
```

从当前代码可以看到几个关键落实点：

- `main.c` 只包含项目自己的公共头，不直接包含 HAL
- `app.h`、`services.h`、`drivers.h` 不包含 `stm32f1xx_hal.h`
- HAL 只在 `platform/` 内部和 `user/stm32f1xx_hal_conf.h` 中出现

这还不是终点，但已经从“目录分层”迈向了“依赖边界分层”。

## 当前骨架的边界

为了保持首轮重构稳定，当前版本有意不做以下内容：

- 不提供真实板卡资源表
- 不引入 LED/UART/Button 驱动示例
- 不引入日志、事件、软件定时器、协议栈
- 不引入业务状态机示例
- 不引入看门狗策略

换句话说，现在的目标是先把“骨架”和“边界”立住，而不是在一个重构提交里把所有产品能力一起塞进去。

## 后续扩展建议

在当前架构上继续扩展时，推荐顺序如下：

1. 先在 `board/custom/` 中补齐板卡资源定义
2. 再在 `drivers/` 中引入具体外设驱动
3. 然后在 `services/` 中承接日志、事件、协议、存储等机制层能力
4. 最后把产品行为、状态机和业务编排放进 `app/`

如果保持这条路径，后续代码规模增长时，结构也会更稳。

## 结论

当前仓库已经不再只是“一个能跑起来的 HAL 模板”，而是一个具备以下特征的分层裸机骨架：

- 启动链和构建链完整
- `user/` 被收缩为兼容桥接层
- `platform/` 成为 HAL 和芯片细节的收口点
- `board/`、`drivers/`、`services/`、`app/` 的职责边界已经立住
- HAL 配置和构建输入已经同步收紧

它现在更适合作为长期维护项目的起点，而不是继续往 `user/` 里堆代码的模板基线。

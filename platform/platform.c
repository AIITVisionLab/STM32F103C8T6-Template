#include "platform.h"

#include "project_config.h"
#include "stm32f1xx_hal.h"

#if PROJECT_TARGET_SYSCLK_HZ != 64000000UL
#error "The current platform clock skeleton expects a 64 MHz SYSCLK target."
#endif

static volatile project_status_t s_last_panic_status = PROJECT_STATUS_OK;

static project_status_t platform_status_from_hal(HAL_StatusTypeDef status);
static project_status_t platform_configure_system_clock(void);

project_status_t platform_init(void)
{
  HAL_StatusTypeDef hal_status = HAL_Init();

  if (hal_status != HAL_OK)
  {
    return platform_status_from_hal(hal_status);
  }

  return platform_configure_system_clock();
}

void platform_poll(void)
{
}

uint32_t platform_get_tick_ms(void)
{
  return HAL_GetTick();
}

uint32_t platform_enter_critical(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();

  return primask;
}

void platform_exit_critical(uint32_t primask)
{
  if ((primask & 1U) == 0U)
  {
    __enable_irq();
  }
}

void platform_panic(project_status_t status)
{
  s_last_panic_status = status;

  __disable_irq();

  while (1)
  {
    __NOP();
  }
}

static project_status_t platform_status_from_hal(HAL_StatusTypeDef status)
{
  switch (status)
  {
  case HAL_OK:
    return PROJECT_STATUS_OK;
  case HAL_ERROR:
    return PROJECT_STATUS_INIT_FAILED;
  case HAL_BUSY:
    return PROJECT_STATUS_TIMEOUT;
  case HAL_TIMEOUT:
    return PROJECT_STATUS_TIMEOUT;
  default:
    return PROJECT_STATUS_UNSUPPORTED;
  }
}

static project_status_t platform_configure_system_clock(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};

  oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  oscinitstruct.HSEState = RCC_HSE_OFF;
  oscinitstruct.LSEState = RCC_LSE_OFF;
  oscinitstruct.HSIState = RCC_HSI_ON;
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL16;

  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
  {
    return PROJECT_STATUS_INIT_FAILED;
  }

  clkinitstruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                            RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
  {
    return PROJECT_STATUS_INIT_FAILED;
  }

  return PROJECT_STATUS_OK;
}

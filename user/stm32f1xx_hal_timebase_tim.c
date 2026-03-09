#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

static TIM_HandleTypeDef g_tim2_timebase;

static uint32_t GetTim2ClockHz(void)
{
  RCC_ClkInitTypeDef clk_config = {0};
  uint32_t flash_latency = 0U;
  uint32_t pclk1 = 0U;

  HAL_RCC_GetClockConfig(&clk_config, &flash_latency);
  pclk1 = HAL_RCC_GetPCLK1Freq();

  if (clk_config.APB1CLKDivider == RCC_HCLK_DIV1)
  {
    return pclk1;
  }

  return pclk1 * 2U;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  uint32_t tim2_clock_hz = 0U;
  uint32_t prescaler = 0U;
  uint32_t period = 0U;

  if (TickPriority >= (1UL << __NVIC_PRIO_BITS))
  {
    return HAL_ERROR;
  }

  __HAL_RCC_TIM2_CLK_ENABLE();

  g_tim2_timebase.Instance = TIM2;
  if (g_tim2_timebase.State != HAL_TIM_STATE_RESET)
  {
    (void) HAL_TIM_Base_Stop_IT(&g_tim2_timebase);
  }

  tim2_clock_hz = GetTim2ClockHz();
  prescaler = (tim2_clock_hz / 1000000U) - 1U;
  period = (1000000U / (1000U / (uint32_t) uwTickFreq)) - 1U;

  g_tim2_timebase.Init.Period = period;
  g_tim2_timebase.Init.Prescaler = prescaler;
  g_tim2_timebase.Init.ClockDivision = 0U;
  g_tim2_timebase.Init.CounterMode = TIM_COUNTERMODE_UP;
  g_tim2_timebase.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&g_tim2_timebase) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_NVIC_SetPriority(TIM2_IRQn, TickPriority, 0U);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  uwTickPrio = TickPriority;

  return HAL_TIM_Base_Start_IT(&g_tim2_timebase);
}

void HAL_SuspendTick(void)
{
  __HAL_TIM_DISABLE_IT(&g_tim2_timebase, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
  __HAL_TIM_ENABLE_IT(&g_tim2_timebase, TIM_IT_UPDATE);
}

void HAL_Delay(uint32_t Delay)
{
  if ((xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) &&
      (__get_IPSR() == 0U))
  {
    TickType_t ticks = 0U;

    if (Delay != 0U)
    {
      ticks = (TickType_t) (Delay / portTICK_PERIOD_MS);
      if ((Delay % portTICK_PERIOD_MS) != 0U)
      {
        ticks++;
      }

      if (ticks != 0U)
      {
        vTaskDelay(ticks);
      }
    }

    return;
  }

  {
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = Delay;

    if (wait < HAL_MAX_DELAY)
    {
      wait += (uint32_t) uwTickFreq;
    }

    while ((HAL_GetTick() - tickstart) < wait)
    {
    }
  }
}

void TIM2_IRQHandler(void)
{
  if ((__HAL_TIM_GET_FLAG(&g_tim2_timebase, TIM_FLAG_UPDATE) != RESET) &&
      (__HAL_TIM_GET_IT_SOURCE(&g_tim2_timebase, TIM_IT_UPDATE) != RESET))
  {
    __HAL_TIM_CLEAR_IT(&g_tim2_timebase, TIM_IT_UPDATE);
    HAL_IncTick();
  }
}

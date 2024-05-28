/*
 * MIT License
 *
 * Copyright (c) 2024 Surya Poudel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SANO_RTOS_SCHEDULER_H
#define __SANO_RTOS_SCHEDULER_H

#include "osConfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef PLATFORM_STM32
#define SYSTICK_HANDLER osSysTick_Handler
/*For STM32 SoCs, SysTick timer is initialized during ClockConfig stage;
 Hence, we dont need it re-initialize SysTick timer for STM32 platform.*/
#define SYSTICK_CONFIG() ((void)0)
    void osSysTick_Handler();
#else
#define SYSTICK_HANDLER SysTick_Handler
#define SYSTICK_CONFIG() SysTick_Config(OS_INTERVAL_CPU_TICKS)
#endif

    void schedulerStart();

    void taskYield();

#ifdef __cplusplus
}
#endif

#endif

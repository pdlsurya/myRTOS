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

#ifndef __SANO_RTOS_TASK_H
#define __SANO_RTOS_TASK_H

#include "osConfig.h"
#include "taskQueue/taskQueue.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TASK_LOWEST_PRIORITY 0xff
#define TASK_HIGHEST_PRIORITY 0

#define TASK_NO_WAIT 0
#define TASK_MAX_WAIT 0xffffffffUL

/**
 * @brief Statically define and initialize a task.
 * @param name Name of the task.
 * @param stackSize Size of task stack in bytes.
 * @param taskEntryFunction Task  entry  function.
 * @param taskParams Entry point parameter.
 * @param taskPriority Task priority.
 */
#define TASK_DEFINE(name, stackSize, taskEntryFunction, taskParams, taskPriority)    \
    void taskEntryFunction(void *);                                                  \
    uint32_t name##Stack[stackSize / sizeof(uint32_t)] = {0};                        \
    taskHandleType name = {                                                          \
        .stackPointer = (uint32_t)(name##Stack + stackSize / sizeof(uint32_t) - 17), \
        .priority = taskPriority,                                                    \
        .taskEntry = taskEntryFunction,                                              \
        .params = taskParams,                                                        \
        .remainingSleepTicks = 0,                                                    \
        .status = TASK_STATUS_READY,                                                 \
        .blockedReason = BLOCK_REASON_NONE,                                          \
        .wakeupReason = WAKEUP_REASON_NONE}

    typedef void (*taskFunctionType)(void *params);

    typedef enum
    {
        TASK_STATUS_READY,
        TASK_STATUS_RUNNING,
        TASK_STATUS_BLOCKED,
        TASK_STATUS_SUSPENDED
    } taskStatusType;

    typedef enum
    {
        BLOCK_REASON_NONE,
        SLEEP,
        WAIT_FOR_SEMAPHORE,
        WAIT_FOR_MUTEX,
        WAIT_FOR_MSG_QUEUE_DATA,
        WAIT_FOR_MSG_QUEUE_SPACE,
        WAIT_FOR_COND_VAR,
        WAIT_FOR_TIMER_TIMEOUT,

    } blockedReasonType;

    typedef enum
    {
        WAKEUP_REASON_NONE,
        WAIT_TIMEOUT,
        SLEEP_TIME_TIMEOUT,
        SEMAPHORE_TAKEN,
        MUTEX_LOCKED,
        MSG_QUEUE_DATA_AVAILABLE,
        MSG_QUEUE_SPACE_AVAILABE,
        COND_VAR_SIGNALLED,
        TIMER_TIMEOUT,
        RESUME

    } wakeupReasonType;

    /*Task control block struct*/
    typedef struct taskHandle
    {
        uint32_t stackPointer;
        taskFunctionType taskEntry;
        void *params;
        uint32_t remainingSleepTicks;
        taskStatusType status;
        blockedReasonType blockedReason;
        wakeupReasonType wakeupReason;
        uint8_t priority;

    } taskHandleType;

    typedef struct
    {
        taskQueueType readyQueue;
        taskQueueType blockedQueue;
        taskHandleType *currentTask;

    } taskPoolType;

    extern taskHandleType *currentTask;
    extern taskHandleType *nextTask;
    extern taskPoolType taskPool;

    void taskStart(taskHandleType *pTask);

    int taskSleepMS(uint32_t sleepTimeMS);

    int taskSleepUS(uint32_t sleepTimeUS);

    void taskSetReady(taskHandleType *pTask, wakeupReasonType wakeupReason);

    void taskBlock(taskHandleType *pTask, blockedReasonType blockedReason, uint32_t ticks);

    void taskSuspend(taskHandleType *pTask);

    int taskResume(taskHandleType *pTask);

#ifdef __cplusplus
}
#endif

#endif

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

#include <stdlib.h>
#include <assert.h>
#include "osConfig.h"
#include "retCodes.h"
#include "task/task.h"
#include "scheduler/scheduler.h"
#include "taskQueue/taskQueue.h"
#include "mutex.h"

/**
 * @brief Lock/acquire the mutex. Because mutexes incorporate ownership control and
 * priority inheritance, calling this function from an ISR is not allowed.
 * @param pMutex Pointer to the mutex structure
 * @param waitTicks Number of ticks to wait if mutex is not available
 * @retval RET_SUCCESS if mutex locked successfully
 * @retval RET_BUSY  if mutex not available
 * @retval RET_TIMEOUT if timeout occured while waiting for mutex
 */
int mutexLock(mutexHandleType *pMutex, uint32_t waitTicks)
{
    assert(pMutex != NULL);

    int retCode;

    ENTER_CRITICAL_SECTION();

    taskHandleType *currentTask = taskPool.currentTask;
#if MUTEX_USE_PRIORITY_INHERITANCE
    /* Priority inheritance*/
    if (pMutex->ownerTask && currentTask->priority < pMutex->ownerTask->priority)
    {
        /* Save owner task's default priority if not saved before */
        if (pMutex->ownerDefaultPriority == -1)
        {
            pMutex->ownerDefaultPriority = pMutex->ownerTask->priority;
        }
        pMutex->ownerTask->priority = currentTask->priority;
    }
#endif
    /* Check if mutex is free and no owner has been assigned. If so, lock mutex immediately.*/
    if (!pMutex->locked)
    {
        pMutex->locked = true;
        pMutex->ownerTask = currentTask;

        retCode = RET_SUCCESS;
    }

    else if (waitTicks == TASK_NO_WAIT && pMutex->locked)
    {
        retCode = RET_BUSY;
    }

    else
    {
        /* Add the tasking waiting on mutex to the wait queue*/
        taskQueueAdd(&pMutex->waitQueue, currentTask);

        /*Exit from critical section before blocking the task*/
        EXIT_CRITICAL_SECTION();

        /* Block current task and give CPU to other tasks while waiting for mutex*/
        taskBlock(currentTask, WAIT_FOR_MUTEX, waitTicks);

        /*Re-enter critical section after being unblocked*/
        ENTER_CRITICAL_SECTION();

        if (currentTask->wakeupReason == MUTEX_LOCKED && pMutex->ownerTask == currentTask)
        {
            retCode = RET_SUCCESS;
        }
        else
        {
            retCode = RET_TIMEOUT;
        }
    }
    EXIT_CRITICAL_SECTION();

    return retCode;
}

/**
 * @brief Unlock/Release mutex.Because mutexes incorporate ownership control and
 * priority inheritance, calling this function from an ISR is not allowed.
 * @param pMutex Pointer to the mutex structure
 * @retval RET_SUCCESS if mutex unlocked successfully
 * @retval RET_NOTOWNER if current owner doesnot owns the mutex
 * @retval RET_NOTLOCKED if mutex was not previously locked
 */
int mutexUnlock(mutexHandleType *pMutex)
{

    assert(pMutex != NULL);

    int retCode;

    bool contextSwitchRequired = false;

    ENTER_CRITICAL_SECTION();

    taskHandleType *currentTask = taskPool.currentTask;

    /*Unlocking the mutex is possible only if current task owns it*/

    if (pMutex->ownerTask == currentTask)
    {
        if (pMutex->locked)
        {

#if MUTEX_USE_PRIORITY_INHERITANCE
            /* Assign owner task its default priority if priority inheritance was perforemd while locking the mutex*/
            if (pMutex->ownerDefaultPriority != -1)
            {
                pMutex->ownerTask->priority = pMutex->ownerDefaultPriority;

                /* Reset owner defalult priority of mutex*/
                pMutex->ownerDefaultPriority = -1;
            }
#endif
            /* Get next owner of the mutex*/
            taskHandleType *nextOwner = taskQueueGet(&pMutex->waitQueue);

            pMutex->ownerTask = nextOwner;

            if (nextOwner != NULL)
            {
                taskSetReady(nextOwner, MUTEX_LOCKED);

                /*Perform context switch if next owner task has equal or
                 *higher priority[lower priority value] than that of current task */
                if (nextOwner->priority <= taskPool.currentTask->priority)
                {
                    contextSwitchRequired = true;
                }
            }
            else
            {
                pMutex->locked = false;
            }

            retCode = RET_SUCCESS;
        }
        else
        {
            retCode = RET_NOTLOCKED;
        }
    }
    else
    {
        retCode = RET_NOTOWNER;
    }

    EXIT_CRITICAL_SECTION();

    if (contextSwitchRequired)
    {
        taskYield();
    }

    return retCode;
}

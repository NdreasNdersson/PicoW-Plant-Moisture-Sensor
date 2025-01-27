#include "run_time_stats.h"

#include <cstdio>

#include "FreeRTOS.h"
#include "task.h"

void runTimeStats() {
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    configRUN_TIME_COUNTER_TYPE ulTotalRunTime;

    /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
    uxArraySize = uxTaskGetNumberOfTasks();
    printf("Number of tasks %u\n", uxArraySize);

    /* Allocate a TaskStatus_t structure for each task.  An array could be
    allocated statically at compile time. */
    pxTaskStatusArray =
        (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != nullptr) {
        /* Generate raw status information about each task. */
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,
                                           &ulTotalRunTime);

        /* For each populated position in the pxTaskStatusArray array,
        format the raw data as human readable ASCII data. */
        for (x = 0; x < uxArraySize; x++) {
            printf("Task: %u \t cPri:%u \t bPri:%u \t hw:%u \t%s\n",
                   pxTaskStatusArray[x].xTaskNumber,
                   pxTaskStatusArray[x].uxCurrentPriority,
                   pxTaskStatusArray[x].uxBasePriority,
                   pxTaskStatusArray[x].usStackHighWaterMark,
                   pxTaskStatusArray[x].pcTaskName);
        }

        /* The array is no longer needed, free the memory it consumes. */
        vPortFree(pxTaskStatusArray);
    } else {
        printf(("Failed to allocate space for stats\n"));
    }

    HeapStats_t heapStats;
    vPortGetHeapStats(&heapStats);
    printf("HEAP avl: %zu, blocks %zu, alloc: %zu, free: %zu\n",
           heapStats.xAvailableHeapSpaceInBytes, heapStats.xNumberOfFreeBlocks,
           heapStats.xNumberOfSuccessfulAllocations,
           heapStats.xNumberOfSuccessfulFrees);
}

#include "run_time_stats.h"

#include "FreeRTOS.h"
#include "task.h"
#include "utils/logging.h"

void runTimeStats() {
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    unsigned long ulTotalRunTime;

    /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
    uxArraySize = uxTaskGetNumberOfTasks();
    LogInfo(("Number of tasks %d\n", uxArraySize));

    /* Allocate a TaskStatus_t structure for each task.  An array could be
    allocated statically at compile time. */
    pxTaskStatusArray =
        (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {
        /* Generate raw status information about each task. */
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,
                                           &ulTotalRunTime);

        /* For each populated position in the pxTaskStatusArray array,
        format the raw data as human readable ASCII data. */
        for (x = 0; x < uxArraySize; x++) {
            LogInfo(("Task: %d \t cPri:%d \t bPri:%d \t hw:%d \t%s\n",
                     pxTaskStatusArray[x].xTaskNumber,
                     pxTaskStatusArray[x].uxCurrentPriority,
                     pxTaskStatusArray[x].uxBasePriority,
                     pxTaskStatusArray[x].usStackHighWaterMark,
                     pxTaskStatusArray[x].pcTaskName));
        }

        /* The array is no longer needed, free the memory it consumes. */
        vPortFree(pxTaskStatusArray);
    } else {
        LogInfo(("Failed to allocate space for stats\n"));
    }

    HeapStats_t heapStats;
    vPortGetHeapStats(&heapStats);
    LogInfo(("HEAP avl: %d, blocks %d, alloc: %d, free: %d\n",
             heapStats.xAvailableHeapSpaceInBytes,
             heapStats.xNumberOfFreeBlocks,
             heapStats.xNumberOfSuccessfulAllocations,
             heapStats.xNumberOfSuccessfulFrees));
}

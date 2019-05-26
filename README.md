AMBLAQ - Another Multithreaded Bounded Lockfree Atomic Queue
============================================================
By Richard Maxwell

This is a single header style, C11/C++11, macro based, strongly typed,
runtime sized, multithreaded, bounded, lockfree, atomic queue fifo based on the
work by Dmitry Vyukov.

(http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue)

Example Usage
-------------

```c
#include <stdint.h>
#include <malloc.h>

#define MY_STRUCT_DATA_COUNT 8

typedef struct My_Struct
{
    uint64_t tag;
    uint8_t  data[MY_STRUCT_DATA_COUNT];
}
My_Struct;

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE My_Struct
#define QUEUE_IMPLEMENTATION
#include <amblaq/queues.h>

int example()
{
    // create the queue.
    Queue_Spsc_My_Struct* queue;
    size_t bytes = 0;

    if (spsc_make_queue_My_Struct(256, NULL, &bytes) != Queue_Result_Ok)
    {
        return 1;
    }

    queue = (Queue_Spsc_My_Struct*) malloc(bytes);

    if (spsc_make_queue_My_Struct(256, queue, &bytes) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // Queue
    My_Struct reference =
    {
          22
        , {1,2,3,4,5,6,7,8}
    };

    if (spsc_enqueue_My_Struct(queue, &reference) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // de-queue
    My_Struct result;

    if (spsc_dequeue_My_Struct(queue, &result) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // It's the same right?
    if (result.tag != reference.tag)
    {
        free(queue);
        return 2;
    }

    for (size_t i = 0; i < MY_STRUCT_DATA_COUNT; i++)
    {
        if (result.data[i] != reference.data[i])
        {
            free(queue);
            return 2;
        }
    }

    free(queue);
    return 0;
}
```

Status
------
v0.1.0: Work in progress

License
-------
The code is licesned under the MIT License. See [LICENSE].

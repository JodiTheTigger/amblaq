AMBLAQ - Another Multithreaded Bounded Lockfree Atomic Queue
============================================================
By Richard Maxwell

This is a single header style, C11/C++11, macro based, strongly typed,
runtime sized, multithreaded, bounded, lockfree, atomic queue fifo based on the
work by Dmitry Vyukov.

(http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue)

[![Build Status](https://travis-ci.org/JodiTheTigger/amblaq.svg?branch=master)](https://travis-ci.org/JodiTheTigger/amblaq)

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

### Example as a header + c file

my_struct.h
```c
#ifndef MY_STRUCT_H
#define MY_STRUCT_H

#define MY_STRUCT_DATA_COUNT 8

typedef struct My_Struct
{
    uint64_t tag;
    uint8_t  data[MY_STRUCT_DATA_COUNT];
}
My_Struct;

#endif // MY_STRUCT_H
```

my_queue.h:
```c
#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include "my_struct.h"

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE My_Struct
#include <amblaq/queues.h>

#endif // MY_QUEUE_H
```

my_queue.c:
```c
#define QUEUE_IMPLEMENTATION

// NOTE: Cannot include my_queue.h as that could result in missing
//       or duplicate symbols depending on header include order.

#include "my_struct.h"

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE My_Struct
#include <amblaq/queues.h>
```

Status
------
* Tested on Linux, Mac
* Supported on windows if compiled as C++, but untested

Development Notes
-----------------
I wanted a type strong queue system in C, and due to this the entire queue
became an x-macro. I don't like this, as it's hard to use, and while most
IDEs can do code completion with the file, it's still hard to browse.

The alternative (and standard in C it seems) is to strip the type and use void
pointers instead. That way the file would become a normal c file, but you would
lose type safety.

I also decided having the size of the queue defined at runtime. This was a weak
decision. If it was set at compile time, then the queue structure could be known
at compile time and be instantated on the stack. It would also make the setup
code far simpler.

I chose one file instead of four files (for spsc, spmc, mpsc and mpmc) as the
code was near identical except for a few lines. The was not a strong decision
as I still don't like all the macro code resulting in this for readability
reasons.

Using the C stdlib for test thread code was a pain as I discovered hardly any
stdlibs support it. I could have used a wrapper library to simulate it but It
was easier to disable the test instead.

I gave up on doing a windows version test for a few reasons:
* msvc compiles C code as C89/C90 therefore cannot compile amblaq
* therefore I need to change the cmake to build it as C++
* msvc still reports C++ as pre C++11 with the __cplusplus macro even though
  it _does_ support C++11 (needs a special flag for it to report as expected)
* I would have to amend my test code to use C++11 as well and I'm tired and
  can't be bothered

I think writing this code in C was a mistake, but I do love the compiler speed.

License
-------
The code is licesned under the MIT License. See [LICENSE].

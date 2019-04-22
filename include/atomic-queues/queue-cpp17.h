#ifndef QUEUE_CPP17_H
#define QUEUE_CPP17_H

#include <cstdint>
#include <atomic>
#include <array>

namespace atomic_queues
{

enum class Result
{
      Ok
    , Full
    , Empty
    , Contention
    , Error
};

template
<
      typename DATA_TYPE        = std::uint64_t
    , int      CELL_COUNT       = 1024
    , int      SINGLE_CONSUMER  = 1
    , int      SINGLE_PRODUCER  = 1
    , int      CACHELINE_BYTES  = 64
>
class Queue_Cpp17 final
{
    using value_type = DATA_TYPE;

    using Enqueue_Index = typename std::conditional
    <
          SINGLE_PRODUCER == 0
        , std::size_t
        , std::atomic_size_t
    >
    ::type;

    using Dequeue_Index = typename std::conditional
    <
          SINGLE_CONSUMER == 0
        , std::size_t
        , std::atomic_size_t
    >
    ::type;


    static_assert
    (
          CELL_COUNT > 1
        , "CELL_COUNT_POW2 must be 2 or larger"
    );

    static_assert
    (
          (CELL_COUNT & (CELL_COUNT - 1)) == 0
        , "CELL_COUNT_POW2 must be a power of 2"
    );

    Queue_Cpp17()
    {
        for (size_t i = 0; i < CELL_COUNT; i++)
        {
            cells[i].sequence.store(i);
        }
    }

    Result try_enqueue(DATA_TYPE data)
    {
        std::size_t pos;
        if constexpr(SINGLE_PRODUCER)
        {
            pos = enqueue_index;
        }
        else
        {
            pos = enqueue_index.load(std::memory_order_relaxed);
        }

        Cell* cell = &cells[pos & cell_mask];

        size_t sequence = cell->sequence.load(std::memory_order_acquire);

        intptr_t difference =
            static_cast<intptr_t>(sequence) - static_cast<intptr_t>(pos);

        if (!difference)
        {
            if constexpr(SINGLE_PRODUCER)
            {
                enqueue_index++;
                cell->data = data;

                return Result::Ok;
            }
            else
            {
                if
                 (
                    enqueue_index.compare_exchange_weak
                    (
                          pos
                        , pos + 1
                        , std::memory_order_relaxed
                        , std::memory_order_relaxed
                    )
                )
                {
                    cell->data = *data;

                    cell->sequence.store(pos + 1, std::memory_order_release);

                    return Result::Ok;
                }
            }
        }

        if (difference < 0)
        {
            return Result::Full;
        }

        return Result::Contention;
    }

    Result try_dequeue(DATA_TYPE* data) // ram: todo: dont return via pointer
    {
        std::size_t pos = dequeue_index.load(std::memory_order_relaxed);

        Cell* cell = &cells[pos & cell_mask];

        size_t sequence = cell->sequence.load(std::memory_order_acquire);

        intptr_t difference =
            static_cast<intptr_t>(sequence) - static_cast<intptr_t>(pos + 1);

        if (!difference)
        {
            if
            (
                dequeue_index.compare_exchange_weak
                (
                      pos
                    , pos + 1
                    , std::memory_order_relaxed
                    , std::memory_order_relaxed
                )
            )
            {
                *data = cell->data;

                cell->sequence.store
                (
                      pos + cell_mask + 1
                    , std::memory_order_release
                );

                return Result::Ok;
            }
        }

        if (difference < 0)
        {
            return Result::Empty;
        }

        return Result::Contention;
    }

private:
    struct Cell
    {
        std::atomic_size_t sequence;
        DATA_TYPE          data;
    };

    uint8_t             pad0[CACHELINE_BYTES];

    Enqueue_Index       enqueue_index = 0;
    uint8_t             pad2[CACHELINE_BYTES - sizeof(Enqueue_Index)];

    Dequeue_Index       dequeue_index = 0;
    uint8_t             pad3[CACHELINE_BYTES - sizeof(Dequeue_Index)];

    size_t              cell_mask = CELL_COUNT - 1;
    uint8_t             pad4[CACHELINE_BYTES - sizeof(size_t)];

    std::array<Cell, CELL_COUNT> cells;

    // -------------------------------------------------------------------------
};


}


#endif // QUEUE_CPP17_H

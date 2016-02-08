//----------------------------------------------------------------
// Statically-allocated memory manager
//
// by Eli Bendersky (eliben@gmail.com)
//  
// This code is in the public domain.
//----------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>

#include "private/debug.h"

#ifndef STACK_SIZE
#define STACK_SIZE 512
#endif

#define MIN_POOL_ALLOC_QUANTAS 16
#define CANARY_VALUE 0xbabe

typedef uint16_t Canary;

union mem_header_union
{
    struct 
    {
        // Pointer to the next block in the free list
        //
        union mem_header_union* next;

        // Size of the block (in quantas of sizeof(mem_header_t))
        //
        size_t size;
    } s;

    // Used to align headers in memory to a boundary
    //
    size_t align_dummy;
};

typedef union mem_header_union mem_header_t;

// Initial empty list
//
static mem_header_t base = {.s = {0, 0}};

// Start of free list
//
static mem_header_t* freep = 0;

// Static pool for new allocations
//
extern char __bss_end;
extern char __stack;

#define POOL_SIZE ((uintptr_t)&__stack - STACK_SIZE - (uintptr_t)&__bss_end - \
                   sizeof(Canary))

static uint8_t* pool = (uint8_t*)&__bss_end;
static size_t pool_free_pos = 0;
static Canary* canary;

#ifdef TRACE_MALLOC
static volatile int do_trace = 1;

#define START_TRACING() do {do_trace = 1;} while(0)
#define STOP_TRACING()  do {do_trace = 0;} while(0)

typedef struct
{
    void*  location;
    size_t size;
    void*  owner;
} Allocation;

static Allocation allocations[TRACE_MALLOC_NUM_ALLOCS] = {{0, 0, 0},};
#else
#define START_TRACING()
#define STOP_TRACING()
#endif

void free(void* ap);

void __attribute__((constructor)) memmgr_init()
{
    canary = (Canary*)((uintptr_t)&__stack - STACK_SIZE - sizeof(Canary));
    *canary = CANARY_VALUE;
}

static void check_canary()
{
    if (*canary != CANARY_VALUE)
        puts("WARNING: stack overflow detected");
}

void memmgr_print_stats()
{
#ifdef TRACE_MALLOC
    mem_header_t* p;

    printf("------ Memory manager stats ------\n\n");
    printf(    "Pool: free_pos = %u (%u bytes left)\n\n",
            pool_free_pos, POOL_SIZE - pool_free_pos);

    p = (mem_header_t*) pool;

    while (p < (mem_header_t*) (pool + pool_free_pos))
    {
        printf(    "  * Addr: %p; Size: %8u\n",
                p, p->s.size);

        p += p->s.size;
    }

    printf("\nFree list:\n\n");

    if (freep)
    {
        p = freep;

        while (1)
        {
            printf(    "  * Addr: %p; Size: %8u; Next: %p\n",
                    p, p->s.size, p->s.next);

            p = p->s.next;

            if (p == freep)
                break;
        }
    }
    else
    {
        printf("Empty\n");
    }

    printf("\n");
#endif
}

void memmgr_print_allocations()
{
#ifdef TRACE_MALLOC
    printf("------ Current allocations ------\n");

    for (size_t i = 0; i < sizeof(allocations) / sizeof(Allocation); i++)
    {
        Allocation* allocation = &allocations[i];

        if (allocation->location != NULL)
        {
            printf("Pointer: %p, size: %u, owner: %p\n",
                   allocation->location, allocation->size, allocation->owner);
        }
    }

    printf("---------------------------------\n");
#endif
}

#ifdef TRACE_MALLOC
static void add_allocation(void* location, size_t size, void* caller)
{
    if (!do_trace)
        return;

    printf("%p: malloc(%u) -> %p\n", caller, size, location);

    for (size_t i = 0; i < sizeof(allocations) / sizeof(Allocation); i++)
    {
        Allocation* allocation = &allocations[i];

        if (allocation->location == NULL)
        {
            allocation->location = location;
            allocation->size = size;
            allocation->owner = caller;
            return;
        }
    }

    puts("TRACE_MALLOC: max number of traced allocations reached");
}

static void remove_allocation(void* location, void* caller)
{
    if (!do_trace)
        return;

    printf("%p: free(%p)\n", caller, location);

    if (location == NULL)
        return;

    for (size_t i = 0; i < sizeof(allocations) / sizeof(Allocation); i++)
    {
        Allocation* allocation = &allocations[i];

        if (allocation->location == location)
        {
            allocation->location = NULL;
            allocation->size = 0;
            allocation->owner = NULL;
            return;
        }
    }

    printf("TRACE_MALLOC: no allocation found at %p, illegal free?\n",
           location);
}

static inline void* __attribute__((always_inline)) get_caller()
{
    void* return_addr = __builtin_return_address(0);
    return (char*)return_addr - 4;
}
#endif

static mem_header_t* get_mem_from_pool(size_t nquantas)
{
    size_t total_req_size;

    mem_header_t* h;

    if (nquantas < MIN_POOL_ALLOC_QUANTAS)
        nquantas = MIN_POOL_ALLOC_QUANTAS;

    total_req_size = nquantas * sizeof(mem_header_t);

    if (pool_free_pos + total_req_size <= POOL_SIZE)
    {
        h = (mem_header_t*) (pool + pool_free_pos);
        h->s.size = nquantas;
        STOP_TRACING();
        free((void*) (h + 1));
        START_TRACING();
        pool_free_pos += total_req_size;
    }
    else
    {
        return 0;
    }

    return freep;
}


// Allocations are done in 'quantas' of header size.
// The search for a free block of adequate size begins at the point 'freep' 
// where the last block was found.
// If a too-big block is found, it is split and the tail is returned (this 
// way the header of the original needs only to have its size adjusted).
// The pointer returned to the user points to the free space within the block,
// which begins one quanta after the header.
//
void* malloc(size_t nbytes)
{
    check_canary();

    mem_header_t* p;
    mem_header_t* prevp;

    // Calculate how many quantas are required: we need enough to house all
    // the requested bytes, plus the header. The -1 and +1 are there to make sure
    // that if nbytes is a multiple of nquantas, we don't allocate too much
    //
    size_t nquantas = (nbytes + sizeof(mem_header_t) - 1) / sizeof(mem_header_t) + 1;

    // First alloc call, and no free list yet ? Use 'base' for an initial
    // denegerate block of size 0, which points to itself
    // 
    if ((prevp = freep) == 0)
    {
        base.s.next = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.next; ; prevp = p, p = p->s.next)
    {
        // big enough ?
        if (p->s.size >= nquantas) 
        {
            // exactly ?
            if (p->s.size == nquantas)
            {
                // just eliminate this block from the free list by pointing
                // its prev's next to its next
                //
                prevp->s.next = p->s.next;
            }
            else // too big
            {
                p->s.size -= nquantas;
                p += p->s.size;
                p->s.size = nquantas;
            }

            freep = prevp;
            void* ret = p + 1;

#ifdef TRACE_MALLOC
            add_allocation(ret, nbytes, get_caller());
#endif

            return ret;
        }
        // Reached end of free list ?
        // Try to allocate the block from the pool. If that succeeds,
        // get_mem_from_pool adds the new block to the free list and
        // it will be found in the following iterations. If the call
        // to get_mem_from_pool doesn't succeed, we've run out of
        // memory
        //
        else if (p == freep)
        {
            if ((p = get_mem_from_pool(nquantas)) == 0)
            {
                printf("!! Memory allocation of %uB failed !!\n", nbytes);
                memmgr_print_stats();
                memmgr_print_allocations();
                return 0;
            }
        }
    }
}


// Scans the free list, starting at freep, looking the the place to insert the 
// free block. This is either between two existing blocks or at the end of the
// list. In any case, if the block being freed is adjacent to either neighbor,
// the adjacent blocks are combined.
//
void free(void* ap)
{
#ifdef TRACE_MALLOC
    remove_allocation(ap, get_caller());
#endif

    check_canary();

    if (ap == NULL)
        return;

    mem_header_t* block;
    mem_header_t* p;

    // acquire pointer to block header
    block = ((mem_header_t*) ap) - 1;

    // Find the correct place to place the block in (the free list is sorted by
    // address, increasing order)
    //
    for (p = freep; !(block > p && block < p->s.next); p = p->s.next)
    {
        // Since the free list is circular, there is one link where a 
        // higher-addressed block points to a lower-addressed block. 
        // This condition checks if the block should be actually 
        // inserted between them
        //
        if (p >= p->s.next && (block > p || block < p->s.next))
            break;
    }

    // Try to combine with the higher neighbor
    //
    if (block + block->s.size == p->s.next)
    {
        block->s.size += p->s.next->s.size;
        block->s.next = p->s.next->s.next;
    }
    else
    {
        block->s.next = p->s.next;
    }

    // Try to combine with the lower neighbor
    //
    if (p + p->s.size == block)
    {
        p->s.size += block->s.size;
        p->s.next = block->s.next;
    }
    else
    {
        p->s.next = block;
    }

    freep = p;
}

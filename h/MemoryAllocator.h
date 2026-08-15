
#ifndef PROJECT_BASE_V1_1_MEMORYALLOCATOR_H
#define PROJECT_BASE_V1_1_MEMORYALLOCATOR_H

#include "../lib/hw.h"

class MemoryAllocator{
public:
    static void* alloc(size_t sizeInBlocks);
    static int free(void* ptr);
private:
    struct FreeSegment{
        size_t size;
        FreeSegment* next;
    };
    struct UsedHeader{
        size_t size;
    };

    static FreeSegment* freeListHead;
    static bool initialized;
    static void initialize();
    static int tryToJoin(FreeSegment* s);

};


#endif //PROJECT_BASE_V1_1_MEMORYALLOCATOR_H

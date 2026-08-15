#include "../h/MemoryAllocator.h"
MemoryAllocator::FreeSegment * MemoryAllocator::freeListHead = nullptr;
bool MemoryAllocator::initialized = false;
void MemoryAllocator::initialize(){
    freeListHead = (FreeSegment*) HEAP_START_ADDR;
    freeListHead->size= (char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR;
    freeListHead->next = nullptr;
    initialized = true;
}

void* MemoryAllocator::alloc(size_t sizeInBlocks) {
    if(!initialized) initialize();
    if( sizeInBlocks == 0 ) return nullptr;

    size_t needed= sizeInBlocks*MEM_BLOCK_SIZE + MEM_BLOCK_SIZE;

    //first fit
    FreeSegment* prev = nullptr;
    FreeSegment* cur = freeListHead;
    while(cur && cur->size < needed) {
        prev = cur;
        cur = cur->next;
    }
    if(!cur) return nullptr;
    FreeSegment * remainder;
    if(cur->size - needed >= sizeof(FreeSegment)){
        //ostatak segmenta delimo na novi segment
        remainder = (FreeSegment*) ((char*)cur+needed);
        remainder->size = cur->size - needed;
        remainder-> next = cur->next;
    }else {
        //ako je ostatak premali za zaglavlje
        needed = cur->size;
        remainder = cur->next;
    }
    if(prev) prev->next = remainder;
    else freeListHead = remainder;
    ((UsedHeader*)cur)->size = needed;
    return (char*)cur +MEM_BLOCK_SIZE;
}
int MemoryAllocator::free(void *ptr) {
    if(!ptr || !initialized) return -1;
    if ((char*)ptr < (char*)HEAP_START_ADDR + MEM_BLOCK_SIZE ||
        (char*)ptr >= (char*)HEAP_END_ADDR) return -2;

    //citanje velicine
    FreeSegment* seg = (FreeSegment*) ((char*)ptr - MEM_BLOCK_SIZE);
    size_t size = ((UsedHeader*)seg)->size;

    FreeSegment* cur = nullptr;
    if (freeListHead && (char*)freeListHead <(char*)seg)
        for(cur = freeListHead; cur->next && (char*)cur->next < (char*) seg; cur= cur->next);

    seg->size = size;
    if(cur){
        seg->next =cur->next;
        cur->next = seg;
    }
    else {
        seg->next = freeListHead;
        freeListHead= seg;
    }
    tryToJoin(seg);
    tryToJoin(cur);
    return 0;
}
int MemoryAllocator::tryToJoin(FreeSegment *s) {
    if( !s  || !s->next) return 0;
    if((char*)s + s->size == (char*) s->next){
        s->size += s->next->size;
        s->next = s->next->next;
        return 1;
    }
    return 0;
}
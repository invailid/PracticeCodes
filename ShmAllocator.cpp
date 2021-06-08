#include "ShmAllocator.h"

namespace ShmAllocator 
{
    //initialize static members
    void* MemAlloc::chunk = NULL;   
    int MemAlloc::shmId = 0;
    SemaphoreSet* MemAlloc::sem = NULL;
    bool MemAlloc::freeMemoryFlag = false;
    BlocksList* MemAlloc::bl = NULL;
    
    void MemAlloc::initialize(unsigned sizeBits, bool freeMemory) throw(MemException)
    {
        void* tmpPtr;
        size_t totalSize = 1UL << sizeBits;        
    
        freeMemoryFlag = freeMemory;
    
        //sanity check
        if (sizeBits > 25)
            throw MemException("sizeBite > 25");
    
        //get shared memory block
        shmId = shmget(IPC_PRIVATE, totalSize, (IPC_CREAT | 0600));
        if (shmId == -1)
            throw MemException("Couldn't allocate shared memory");
    
        //attach to block
        chunk = shmat(shmId, 0, 0);
        if ((int)chunk == -1)
        {
            shmctl(shmId, IPC_RMID, 0);
            throw MemException("Couldn't attach to shared memory");
        }
    
        //make sure the block is released when no longer attached
        shmctl(shmId, IPC_RMID, 0);
    
        tmpPtr = chunk;
        new(tmpPtr) SemaphoreSet(1);
        sem = (SemaphoreSet*)tmpPtr;
        (char*)tmpPtr += sizeof(*sem);
        totalSize -= sizeof(*sem);
    
        bl = (BlocksList*)tmpPtr;
        (char*)tmpPtr += sizeof(*bl);
        totalSize -= sizeof(*bl);
        new(bl) BlocksList(totalSize, tmpPtr);
    }
    
    void MemAlloc::destroy()
    {
        sem->~SemaphoreSet();
    
        if (chunk != NULL) {
            shmdt(chunk);
            chunk = NULL;
        }
    }
    
    void* MemAlloc::allocate(size_t size) throw(MemException)
    {
        void* ret;
    
        if (chunk == NULL)
            throw MemException("Uninitialized MemAlloc");
    
        sem->lock(0);
        ret = bl->getBlock(size);
        sem->unlock(0);
    
        if (ret == NULL)
            throw MemException("Out of Memory");
    
        return ret;
    }
    
    void MemAlloc::deallocate(void* p)
    {
        if (freeMemoryFlag)
        {
            sem->lock(0);
            bl->returnBlock(p);
            sem->unlock(0);
        }
    }
}


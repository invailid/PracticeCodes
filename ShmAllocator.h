#ifndef __MYALLOC_H
#define __MYALLOC_H

#include <iostream>
#include <memory>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "SemaphoreSet.h"
#include "ExceptionBase.h"

using namespace std;
using namespace SemSet;

namespace ShmAllocator 
{
    /**
     * Extending ExceptionBase. Handling of memory allocation exceptions.
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class MemException : public ExceptionBase
    {
    public:
        /**
         * Constructor (no explanation string)
         */
        MemException() : ExceptionBase() {}

        /**
         * Constructor
         * 
         * @param mess   The error message
         */
        MemException(const char* mess) : ExceptionBase(mess) {}

        /**
         * Destructor
         */
        ~MemException() {}
    };


    /** Align the memory segment to a 4 bytes multiple*/
    #define ALIGN_SEGMENT(n)    (((n-1)|3)+1)
    /** Returns the maxiumum between a and b */
    #define MAX(a,b)            ((a) >= (b) ? (a) : (b))
    /** Returns the address of a structure of type T whose member A is at address P */
    #define ATTR_TO_TYPE(T,A,P) ((T*)((char*)(P)-offsetof(T,A)))
    /**
     * Class handling memory allocations from a previously
     * allocated memory block.
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class BlocksList
    {
        /**
         * One node in the free memory blocks list.
         */
        struct Node
        {
            Node(const size_t length, Node* next = 0) : length(length), next(next) {}
            size_t length;
            Node* next;
        };
        /**
         * One supplied memory block.
         */
        struct ReturnedBlock
        {
            size_t size;
            char block[0];
        };

    private:
        Node* head;             //the head of the list
        size_t totalSize;       //the total size of the block
        size_t freeSize;        //the free space left
        int numOfBlocks;        //number of nodes in the list
        void* blockStart;       //start address of the block
        void* blockEnd;         //first address not in the block

    public:
        /**
         * Constructor.
         * 
         * @param totalSize  Size of the pre-allocated block.
         * @param blockStart Address of the pre-allocated block.
         */
        BlocksList(size_t totalSize, void* blockStart) : totalSize(totalSize), blockStart(blockStart)
        {
            //create a one node list
            (char*)blockEnd = (char*)blockStart + totalSize;
            freeSize = totalSize;
            head = (Node*)blockStart;
            new(head) Node(freeSize);
            numOfBlocks = 1;
        }

        /**
         * Destructor.
         */
        ~BlocksList() {}

        /**
         * Gets a memory chunk of the requested size.
         * 
         * @param n      The size to allocate.
         * 
         * @return Pointer to the allocated space.
         */
        void* getBlock(size_t n)
        {
            size_t newSize;
            Node* tmpNode;
            Node* prevNode;
            Node* newNext;
            ReturnedBlock* returnedBlock;

            //get the real size to allocate
            n = MAX(ALIGN_SEGMENT(n + sizeof(ReturnedBlock)), sizeof(Node));
            prevNode = NULL;

            //find available space
            for (tmpNode = head; tmpNode; tmpNode = tmpNode->next)
            {
                if (tmpNode->length >= n)
                {
                    //found block
                    returnedBlock = (ReturnedBlock*)tmpNode;
                    freeSize -= n;
                    if ((tmpNode->length > n) && (tmpNode->length - n >= sizeof(Node)))
                    {
                        newSize = tmpNode->length - n;
                        newNext = tmpNode->next;
                        if (prevNode == NULL)
                        {
                            //first link
                            (char*)head = (char*)head + n;
                            new(head) Node(newSize, newNext);
                        }
                        else
                        {
                            //other link
                            (char*)tmpNode = (char*)tmpNode + n;
                            new(tmpNode) Node(newSize, newNext);
                            prevNode->next = tmpNode;
                        }
                    }
                    else 
                    {
                        //use whole block
                        n = tmpNode->length;
                        numOfBlocks--;
                        if (prevNode == NULL)
                            head = head->next;
                        else
                            prevNode->next = tmpNode->next;
                    }
                    //write down the size given and return
                    returnedBlock->size = n;
                    return returnedBlock->block;
                }

                prevNode = tmpNode;
            }

            //found no free space
            return NULL;
        }

        /**
         * Returns space to the block.
         * 
         * @param p      Pointer to the space to return.
         */
        void returnBlock(void *p) throw(MemException)
        {
            ReturnedBlock* returnedBlock;
            void* curBlockEndAddress;
            Node* tmpNode;
            Node* prevNode;
            Node* returnedNode;

            returnedBlock = ATTR_TO_TYPE(ReturnedBlock, block, p);
            (char*)curBlockEndAddress = (char*)returnedBlock + returnedBlock->size;
            //sanity check
            if (((char*)returnedBlock < (char*)blockStart) || 
                ((char*)curBlockEndAddress > (char*)blockEnd))
                throw MemException("Returning memory not allocated");

            freeSize += returnedBlock->size;
            prevNode = NULL;

            //handle adding first node case
            if (head == NULL)
            {
                head = (Node*)returnedBlock;
                new(head) Node(returnedBlock->size);
                return;
            }

            //find the position to keep list sorted
            for (tmpNode = head; tmpNode; tmpNode = tmpNode->next)
            {
                if ((char*)returnedBlock < (char*)tmpNode)
                {
                    returnedNode = (Node*)returnedBlock;
                    numOfBlocks++;
                    if (prevNode == NULL)
                    {
                        //adding before the head
                        new(returnedNode) Node(returnedBlock->size, head);
                        head = returnedNode;
                    }
                    else
                    {
                        //regular add
                        prevNode->next = returnedNode;
                        new(returnedNode) Node(returnedBlock->size, tmpNode);
                    }
                    //connect to previous node if necessary
                    if ((prevNode) && ((char*)prevNode + prevNode->length == (char*)returnedNode))
                    {
                        prevNode->length += returnedNode->length;
                        prevNode->next = returnedNode->next;
                        returnedNode = prevNode;        //to check if we need to connect to next node
                        numOfBlocks--;
                    }
                    //connect to next node if necessary
                    if ((returnedNode->next) && ((char*)returnedNode + returnedNode->length == (char*)returnedNode->next))
                    {
                        returnedNode->length += returnedNode->next->length;
                        returnedNode->next = returnedNode->next->next;
                        numOfBlocks--;
                    }
                    return;
                }

                prevNode = tmpNode;
            }

            //add new last node case
            returnedNode = (Node*)returnedBlock;
            numOfBlocks++;
            prevNode->next = returnedNode;
            new(returnedNode) Node(returnedBlock->size);
            //connect to previous node if necessary
            if ((char*)prevNode + prevNode->length == (char*)returnedNode)
            {
                prevNode->length += returnedNode->length;
                prevNode->next = returnedNode->next;
                numOfBlocks--;
            }
            return;
        }

        /**
         * Accumulates all the free space in the list.
         * 
         * @return The "real" free memory.
         */
        size_t accumFreeMemorySize()
        {
            Node* tmpNode;
            size_t totalFree = 0;

            for (tmpNode = head; tmpNode; tmpNode = tmpNode->next)
                totalFree += tmpNode->length;

            return totalFree;
        }

        /**
         * Gets the total size of the block.
         * 
         * @return The total size of the block.
         */
        size_t getTotalSize()
        {
            return totalSize;
        }

        /**
         * Gets the free size left in the block.
         * 
         * @return The free size left in the block.
         */
        size_t getFreeSize()
        {
            return freeSize;
        }

        /**
         * Gets the number of nodes in the list.
         * 
         * @return The number of nodes in the list.
         */
        size_t getNumOfBlocks()
        {
            return numOfBlocks;
        }
    };


    /**
     * Static class. Handles the shared memory blocks.
     * Performs the actual allocation and deallocation.
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class MemAlloc
    {
    private:
        static void *chunk;         //pointer to the shared memory block
        static int shmId;           //shared memory identifier
        static SemaphoreSet* sem;   //block associated semaphore
        static bool freeMemoryFlag; //indicates whether we want to really free the memory (time consuming)
    
    public:
        static BlocksList* bl;      //free blocks list

        /**
         * Initializes the static class.
         * 
         * @param sizeBits The size of the block will be 2^sizeBits (<=25)
         */
        static void initialize(unsigned sizeBits, bool freeMemory = false) throw(MemException);

        /**
         * Destroy the shared memory block.
         */
        static void MemAlloc::destroy();

        /**
         * Allocates memory from the block.
         * 
         * @param size   Number of bytes to allocate.
         * 
         * @return Pointer to the allocated space.
         */
        static void* allocate(size_t size) throw(MemException);
    
        /**
         * Returns allocated space to the block.
         * 
         * @param p      Pointer to the allocated space to return.
         */
        static void deallocate(void* p);
    };


    /**
     * Generates new objects on the shared memory block.
     *
     * @author Nir Shahaf
     * @version 0.1
     */
    class ObjectFactory
    {
    private:

    public:
        /**
         * Returns a new objects of the requested type.
         * 
         * @param T      Type of object.
         * 
         * @return Pointer to the new object.
         */
        template <typename T>
        static T* getNewObject()
        {
            T* ret = (T*)MemAlloc::allocate(sizeof(T));
            new(ret) T();

            return ret;
        }

        /**
         * Destorys and frees an object.
         * 
         * @param T      The object type.
         * @param obj    The object.
         */
        template <typename T>
        static void returnObject(T* obj)
        {
            obj->~T();
            MemAlloc::deallocate(obj);
        }
    };

    /**
     * The actual STL allocator class.
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    template <typename T>
    class Allocator
    {
    private:
    
    public:
        typedef size_t size_type;               /**< Object size type */
        typedef ptrdiff_t difference_type;      /**< Difference type */
        typedef T* pointer;                     /**< Object pointer type */
        typedef const T* const_pointer;         /**< Const object pointer type */
        typedef T& reference;                   /**< Object reference type */
        typedef const T& const_reference;       /**< Const object reference type */
        typedef T value_type;                   /**< Object type */

        /**
         * rebind (required by the traits class)
         */
        template <typename T1>
            struct rebind
            { 
                typedef Allocator<T1> other; 
            };
    
        /**
         * Empty conructor.
         */
        Allocator() {}

        /**
         * Copy constructor.
         * 
         * @param MyAlloc Another allocator.
         */
        Allocator(const Allocator&) {}

        /**
         * Copy constructor.
         * 
         * @param T1     Allocator type.
         */
        template <typename T1>
            Allocator(const Allocator<T1>&) {}

        /**
         * Destructor.
         */
        ~Allocator() {}
    
        /**
         * Gets address.
         * 
         * @param x      Object reference.
         * 
         * @return Address of object.
         */
        pointer address(reference x) const { return &x; }
    
        /**
         * Gets const address.
         * 
         * @param x      Object reference.
         * 
         * @return Const address of object.
         */
        const_pointer address(const_reference x) const { return &x; }


        /**
         * Allocates new space.
         * 
         * @param n      Number of objects to allocate space for.
         * 
         * @return Pointer to allocated space.
         */
        pointer allocate(size_type n, const void* = 0)
        {
            T* ret = NULL;
            if (n) 
                ret = (pointer)MemAlloc::allocate(n * sizeof(value_type));
            return ret;
        }
    
        /**
         * Returns memory.
         * 
         * @param p      Pointer to free.
         * @param n      Unused (must not be 0)
         */
        void deallocate(pointer p, size_type n)
        {
            if (n != 0)
                MemAlloc::deallocate(p);
        }
    
        /**
         * Gets maximal logical size.
         * 
         * @return The size.
         */
        size_type max_size() const { return size_t(-1) / sizeof(T); }
    
        /**
         * Constructs a new object on a certain memory address (using copy constructor).
         * 
         * @param p      The address.
         * @param val    The value to use for the copy contructor.
         */
        void construct(pointer p, const T& val) { new(p) T(val); }

        /**
         * Destorys an object without freeing memory.
         * 
         * @param p      The object address.
         */
        void destroy(pointer p) { p->~T(); }
    };

}

#endif //__MYALLOC_H

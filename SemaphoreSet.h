#ifndef __SEMAPHORESET_H
#define __SEMAPHOERSET_H

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "ExceptionBase.h"

namespace SemSet
{
    #define ALL_SEMS 0
    
    typedef union semun
    {
        int val;
        struct smid_ds *buf;
        ushort *array;
    } arg;


    /**
     * Handling of semphore set exceptions
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class SemException : public ExceptionBase
    {
    public:
        /**
         * Constructor (no explanation string)
         */
        SemException() : ExceptionBase() {};
    
        /**
         * Constructor
         * 
         * @param mess   The error message
         */
        SemException(const char* mess) : ExceptionBase(mess) {};
    
        /**
         * Destructor
         */
        ~SemException() {};
    };


    /**
     * Handling of unknown semphore set destruction exceptions
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class SemUnknownSetException : public ExceptionBase
    {
    public:
        /**
         * Constructor (no explanation string)
         */
        SemUnknownSetException() : ExceptionBase() {};
    
        /**
         * Constructor
         * 
         * @param mess   The error message
         */
        SemUnknownSetException(const char* mess) : ExceptionBase(mess) {};
    
        /**
         * Destructor
         */
        ~SemUnknownSetException() {};
    };


    /**
     * Set of semaphores.
     * 
     * @author Nir Shahaf
     * @version 0.1
     */
    class SemaphoreSet
    {
    public:
        /**
         * Returns values.
         */
        enum SemSetRetVal { OK, FAIL };
    
        /**
         * Constructor.
         * 
         * @param numOfSems Number of semaphores in the set.
         */
        SemaphoreSet(int numOfSems)
        {
            semSetId = semget(IPC_PRIVATE, numOfSems, IPC_CREAT | 0666);
            if (semSetId == -1) 
            {
                semSetId = FAIL;
                return;
            }

            arg semVal;
            semVal.val = 1;

            for (int i = 0; i < numOfSems; i++)
            {
                int tmpRet = semctl(semSetId, i, SETVAL, semVal);
                if (tmpRet == -1)
                {
                    semSetId = FAIL;
                    destroyAll();
                    return;
                }
            }
        }
    
        /**
         * Copy constructor.
         * 
         * @param otherSet The set to clone.
         */
        SemaphoreSet(const SemaphoreSet &otherSet)
        {
            semSetId = otherSet.semSetId;
        }
    
        /**
         * Destructor.
         */
        ~SemaphoreSet() throw(SemUnknownSetException)
        {
            if (semSetId > 0)
            {
                try 
                {
                    destroyAll();
                }
                catch (SemException &e)
                {
                    throw SemUnknownSetException("SemaphoreSet::~SemaphoreSet : set was probably already destroyed by another process");
                }
            }
        }
    
        /**
         * Locks a semaphore in the set.
         * 
         * @param semNum The id of the semaphore in the set.
         * 
         * @return OK/FAIL
         */
        SemSetRetVal lock(int semNum) throw(SemException)
        {
            struct sembuf op;

            if (semSetId <= 0)
                throw SemException("Semaphore set in wrong condition");

            op.sem_num = semNum;
            op.sem_op = -1;
            op.sem_flg = SEM_UNDO;

            int tmpRet = semop(semSetId, &op, 1);
            if (tmpRet == -1)
                return FAIL;

            return OK;
        }
    
        /**
         * Unlocks a semaphore in the set.
         * 
         * @param semNum The id of the semaphore in the set.
         * 
         * @return OK/FAIL
         */
        SemSetRetVal unlock(int semNum) throw(SemException)
        {
            struct sembuf op;

            if (semSetId <= 0)
                throw SemException("Semaphore set in wrong condition");

            op.sem_num = semNum;
            op.sem_op = 1;
            op.sem_flg = SEM_UNDO;

            int tmpRet = semop(semSetId, &op, 1);
            if (tmpRet == -1)
                return FAIL;

            return OK;
        }
    
    private:
        int semSetId;
    
        SemSetRetVal destroyAll()
        {
            arg semVal; 

            if (semSetId <= 0)
                throw SemException("Semaphore set in wrong condition");

            int tmpRet = semctl(semSetId, ALL_SEMS, IPC_RMID, semVal);
            if (tmpRet == -1)
            {
                semSetId = FAIL;
                throw SemException("Couldn't destroy a semaphore set");
            }
            return OK;
        }
    };
}

#endif //__SEMAPHOERESET_H

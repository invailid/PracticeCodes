#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ShmAllocator.h"

using namespace std;
using namespace ShmAllocator;

typedef map<int, int, less<int>, Allocator<pair<int, int> > > SharedMap;
typedef SharedMap::iterator SharedMapIt;

class CommandLineParameters
{
private:
    bool initialized;
    bool interactive;
    bool ordered;
    bool shortTest;
    
public:
    CommandLineParameters(int argc, char **argv) : initialized(false), interactive(false), ordered(false), shortTest(false)
    {
        int i;

        for (i = 1; i < argc; i++)
        {
            string s(argv[i]);
            if ((s == "--help") || (s == "-h"))
            {
                cout << "Usage: " << argv[0] << " [-i -o -s]" << endl;
                cout << "\ti\t- interactive mode" << endl;
                cout << "\to\t- ordered memory freeing (for automatic mode only)" << endl;
                cout << "\ts\t- short test (for automatic mode only)" << endl;
                initialized = false;
                return;
            }
            else if (s == "-i")
                interactive = true;
            else if (s == "-o")
                ordered = true;
            else if (s == "-s")
                shortTest = true;
            else
            {
                initialized = false;
                return;
            }
        }

        initialized = true;
    }

    bool isInitialized()
    {
        return initialized;
    }

    bool isInteractive()
    {
        return interactive;
    }

    bool isOrdered()
    {
        return ordered;
    }

    bool isShort()
    {
        return shortTest;
    }
};

class InputLinesReader
{
private:
    SharedMap* theMap;
    SemaphoreSet *sem;

public:
    InputLinesReader(SharedMap &newMap) 
    {
        sem = new SemaphoreSet(1);
        theMap = &newMap;
    }

    ~InputLinesReader() 
    {
        try 
        {
            delete sem;
        }
        catch (SemUnknownSetException& e) {}
    }

    int readLine(char* line)
    {
        int numOfWords, key, value;
        char *firstWord, *secondWord, *thirdWord;
        SharedMapIt it;

        numOfWords = 0;
        firstWord = line;

        while ((*line) && (*line != ' '))
            line++;
        numOfWords++;
        if (*line == ' ')
        {
            *line = '\0';
            secondWord = ++line;

            while ((*line) && (*line != ' '))
                line++;
            numOfWords++;
            if (*line == ' ')
            {
                *line = '\0';
                thirdWord = ++line;

                if (*line)
                    numOfWords++;
            }
        }

        switch (firstWord[0])
        {
            case 'A':
            case 'a':
                if (numOfWords < 3)
                    return -1;
                key = atoi(secondWord);
                value = atoi(thirdWord);
                //cout << "inserting " << key << " => " << value << endl;
                sem->lock(0);
                (*theMap)[key] = value;
                sem->unlock(0);
                break;

            case 'D':
            case 'd':
                if (numOfWords < 2)
                    return -1;
                key = atoi(secondWord);
                //cout << "deleting " << key << endl;
                sem->lock(0);
                theMap->erase(key);
                sem->unlock(0);
                break;

            case 'L':
            case 'l':
                if (numOfWords < 2)
                    return -1;
                key = atoi(secondWord);
                //cout << "looking for " << key << endl;
                it = theMap->find(key);
                if (it != theMap->end())
                    cout << "\tfound: " << it->second << endl;
                else
                {
                    cout << "\tnot found" << endl;
                    return -1;
                }
                break;

            case 'P':
            case 'p':
                if (numOfWords < 1)
                    return -1;
                key = atoi(secondWord);
                cout << "printing: " << endl;
                for (it = theMap->begin(); it != theMap->end(); it++)
                    cout << "\t" << it->first << " ==> " << it->second << endl;
                cout << endl;
                break;

            case 'S':
            case 's':
                cout << "\tsize: " << theMap->size() << endl;
                break;

            case 'h':
            case 'H':
            case '?':
                cout << "\ta <key> <value>\t- add key=>value" << endl;
                cout << "\td <key>\t\t- delete key" << endl;
                cout << "\tl <key>\t\t- lookup key" << endl;
                cout << "\tp\t\t- print table" << endl;
                cout << "\ts\t\t- table size" << endl;
                break;

            default:
                return -1;
        }

        return 0;
    }
};

int main(int argc, char **argv)
{
    SharedMap *m;
    char line[100];
    bool stop = false;
    float percentage;

    CommandLineParameters clp(argc, argv);
    if (!clp.isInitialized())
        return 0;

    try
    {
        MemAlloc::initialize(24, true);
        m = ObjectFactory::getNewObject<SharedMap>();
    }
    catch (MemException& e)
    {
        cout << e.getMessage() << endl;
        ObjectFactory::returnObject(m);
        MemAlloc::destroy();
        exit(1);
    }

    InputLinesReader ilr(*m);

    if (clp.isInteractive())
    {
        //interactive test
        while (!stop)
        {
            cin.getline(line, 100);
            if (strlen(line) == 0)
                stop = true;
            else 
                if (ilr.readLine(line) != 0)
                {
                    cerr << "internal error" << endl;
                    stop = true;
                }
        }

        ObjectFactory::returnObject<SharedMap>(m);

        percentage = (float)MemAlloc::bl->getFreeSize() / (float)MemAlloc::bl->getTotalSize() * 100.0;
        cout << endl;
        cout << "total memory: " << MemAlloc::bl->getTotalSize() << endl;
        cout << "free memory: " << MemAlloc::bl->getFreeSize() << " (" << percentage << "%)" << endl;
        cout << "real free memory: " << MemAlloc::bl->accumFreeMemorySize() << endl;
        cout << "number of blocks: " << MemAlloc::bl->getNumOfBlocks() << endl << endl;

        MemAlloc::destroy();
        return 0;
    }

    //automatic test
    string s;
    int modulus, factorI, factorD;
    if (clp.isShort())
    {
        modulus = 6113;
        factorI = 1669;
        factorD = 2036;
    }
    else
    {
        modulus = 200003;
        factorI = 100;
        factorD = 301;
    }
    char tmp1[50], tmp2[50];
    int pid;

    if ((pid = fork()) >= 0)
    {
        for (int i = 1; i < modulus; i++)
        {
            if ((pid == 0) && (i % 2 == 0))
                continue;
            if ((pid != 0) && (i % 2 != 0))
                continue;
            if ((i % 1000 == 0) || ((i - 1) % 1000 == 0))
                cout << "(" << pid << ")" << i << " blocks: " << MemAlloc::bl->getNumOfBlocks() << " free: " << MemAlloc::bl->getFreeSize() << endl;
            sprintf(tmp1, "%d", (i * factorI) % modulus);
            sprintf(tmp2, "%d", i);
            s = "a ";
            s += tmp1;
            s += " ";
            s += tmp2;
            if (ilr.readLine((char*)s.c_str()) != 0)
            {
                cerr << "internal error" << endl;
                break;
            }
        }

        sleep(5);
        ilr.readLine("s");
        if (pid != 0)
        {
            percentage = (float)MemAlloc::bl->getFreeSize() / (float)MemAlloc::bl->getTotalSize() * 100.0;
            cout << endl;
            cout << "total memory: " << MemAlloc::bl->getTotalSize() << endl;
            cout << "free memory: " << MemAlloc::bl->getFreeSize() << " (" << percentage << "%)" << endl;
            cout << "real free memory: " << MemAlloc::bl->accumFreeMemorySize() << endl;
            cout << "number of blocks: " << MemAlloc::bl->getNumOfBlocks() << endl << endl;
        }
        sleep(5);

        if (clp.isOrdered())
        {
            //free in order
            for (int i = 1; i < modulus; i++)
            {
                if ((pid == 0) && (i % 2 != 0))
                    continue;
                if ((pid != 0) && (i % 2 == 0))
                    continue;
                if ((i % 1000 == 0) || ((i - 1) % 1000 == 0))
                    cout << "(" << pid << ")" << i << " blocks: " << MemAlloc::bl->getNumOfBlocks() << " free: " << MemAlloc::bl->getFreeSize() << endl;
                sprintf(tmp1, "%d", (i * factorI) % modulus);
                s = "d ";
                s += tmp1;
                if (ilr.readLine((char*)s.c_str()) != 0)
                {
                    cerr << "internal error" << endl;
                    break;
                }
            }
        }
        else 
        {
            //free out of order
            for (int i = 1; i < modulus; i++)
            {
                if ((pid == 0) && (i % 2 != 0))
                    continue;
                if ((pid != 0) && (i % 2 == 0))
                    continue;
                if ((i % 1000 == 0) || ((i - 1) % 1000 == 0))
                    cout << "(" << pid << ")" << i << " blocks: " << MemAlloc::bl->getNumOfBlocks() << " free: " << MemAlloc::bl->getFreeSize() << endl;
                sprintf(tmp1, "%d", (i * factorD) % modulus);
                s = "d ";
                s += tmp1;
                if (ilr.readLine((char*)s.c_str()) != 0)
                {
                    cerr << "internal error" << endl;
                    break;
                }
            }
        }

        sleep(5);
        ilr.readLine("s");

        if (pid != 0)
        {
            int status;
            waitpid(pid, &status, 0);
        }
        else 
        {
            return 0;
        }
    }

    ObjectFactory::returnObject<SharedMap>(m);

    percentage = (float)MemAlloc::bl->getFreeSize() / (float)MemAlloc::bl->getTotalSize() * 100.0;
    cout << endl;
    cout << "total memory: " << MemAlloc::bl->getTotalSize() << endl;
    cout << "free memory: " << MemAlloc::bl->getFreeSize() << " (" << percentage << "%)" << endl;
    cout << "real free memory: " << MemAlloc::bl->accumFreeMemorySize() << endl;
    cout << "number of blocks: " << MemAlloc::bl->getNumOfBlocks() << endl << endl;

    MemAlloc::destroy();

    return 0;
}

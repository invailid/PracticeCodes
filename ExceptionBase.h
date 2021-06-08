#ifndef __EXCEPTIONBASE_H
#define __EXCEPTIONBASE_H

#include <string>
    
using std::string;
                       
/**
 * Handling of exceptions
 * 
 * @author Nir Shahaf
 * @version 0.1
 */
class ExceptionBase
{
public:
    /**
     * Constructor (no explanation string)
     */
    ExceptionBase() : what() {}

    /**
     * Constructor
     * 
     * @param mess   The error message
     */
    ExceptionBase(const char* mess) : what(mess) {}

    /**
     * Destructor
     */
    ~ExceptionBase() {}

    /**
     * Returns the error string.
     * 
     * @return Reference to the error string.
     */
    string &getMessage() { return what; }

private:
    string what;
};
                          
#endif //__EXCEPTIONBASE_H

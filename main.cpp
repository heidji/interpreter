#include <string>
#include <iostream>
#include <phpcpp.h>
#include "interpreter.h"

using namespace std;

Php::Value interpreter(Php::Parameters &params)
{
    return calc();
}

// Symbols are exported according to the "C" language
extern "C"
{
    // export the "get_module" function that will be called by the Zend engine
    PHPCPP_EXPORT void *get_module()
    {
        // create extension
        static Php::Extension extension("interpreter","1.0");

        // add function, with defined numeric parameters, to extension
        extension.add<interpreter>("interpreter");

        // return the extension module
        return extension.module();
    }
}

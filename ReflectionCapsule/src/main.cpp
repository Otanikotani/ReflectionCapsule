/**
 @file     main.cpp
 @author   Hadzu
 @date     Feb 25, 2012
 @brief    Reflection capsule
 */
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "pocoapp.h"

extern const char* contextString;

POCO_AppContext *
initContext(const char *ctx_string)
{
    POCO_AppEnv *environment = POCO_AppContext::getDefaultAppEnv();
    POCO_AppContext *context = POCO_AppContext::create(ctx_string, "string",
            environment);
    if (context == NULL)
    {
        std::cout << environment->get_message() << std::endl;
        return NULL;
    }
    if (!context->initSingletons())
    {
        std::cout << environment->get_message() << std::endl;
        return NULL;
    }
    return (context);
}

int
main()
{
    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);
    for (int counter = 0; counter < 1; ++counter)
    {
        POCO_AppContext* context = NULL;

        context = initContext(contextString);
        if (NULL == context)
        {
            std::cout << "ApplicationBeans is not set in system configuration"
                    << std::endl;
            return (-1);
        }
        context->initSingletons();
        context->terminate();
        context->destroy();
    }
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;
    printf("Elapsed time: %ld milliseconds\n", mtime);
}

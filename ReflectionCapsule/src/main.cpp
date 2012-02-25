/**
 @file     main.cpp
 @author   Hadzu
 @date     Feb 25, 2012
 @brief    Reflection capsule
 */

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

int main() {
    std::cout << "Hi" << std::endl;

    POCO_AppEnv* environment = new POCO_AppEnv;
    POCO_AppContext::setDefaultAppEnv(environment);
    POCO_AppContext* context = NULL;

    context = initContext(contextString);
    if (NULL == context)
    {
        std::cout << "ApplicationBeans is not set in system configuration" << std::endl;
        return (-1);
    }
    context->initSingletons(environment);
}

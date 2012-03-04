/**
 @file     BeanExample.h
 @author   Hadzu
 @date     Feb 25, 2012
 @brief    Reflection capsule
 */

#ifndef BEANEXAMPLE_H_
#define BEANEXAMPLE_H_

#include <iostream>

class BeanExample
{
public:
    BeanExample();
    virtual
    ~BeanExample();

    void
    test()
    {
//        std::cout << "Test method called" << std::endl;
    }
};

#endif /* BEANEXAMPLE_H_ */

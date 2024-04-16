#ifndef MyComponentImplEx_h
#define MyComponentImplEx_h

#include "MyComponentImpl.h"

class MyComponentImplEx : public MyComponentImpl
{
public:
    MyComponentImplEx();
    ~MyComponentImplEx();
    virtual void go(const Data& d);
};

#endif // MyComponentImplEx_h

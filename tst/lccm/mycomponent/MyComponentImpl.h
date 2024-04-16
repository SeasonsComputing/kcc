#ifndef MyComponentImpl_h
#define MyComponentImpl_h

#include "../IMyComponent.h"

class MyComponentImpl : public IMyComponent 
{
public:
    MyComponentImpl();
    ~MyComponentImpl();
    virtual void go(const Data& d);
};

#endif // MyComponentImpl_h

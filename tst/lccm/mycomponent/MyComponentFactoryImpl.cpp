#include <inc/core/Core.h>
#include "../IMyComponentFactory.h"
#include "MyComponentImpl.h"
#include "MyComponentImplEx.h"

#define KCC_FILE "MyComponentFactoryImpl"

struct MyComponentFactoryImpl : IMyComponentFactory 
{
    MyComponentFactoryImpl()     { kcc::Log::Scope scope(KCC_FILE, "MyComponentFactoryImpl");  kcc::Log::info1("ctor"); }
    ~MyComponentFactoryImpl()    { kcc::Log::Scope scope(KCC_FILE, "~MyComponentFactoryImpl"); kcc::Log::info1("dtor"); }
    IComponent* construct()      { return constructMy(); }
    IMyComponent* constructMy()  { return new MyComponentImpl; }
    IMyComponent* constructMyEx(){ return new MyComponentImplEx; }
};

KCC_COMPONENT_FACTORY_CUST(MyComponentFactoryImpl);

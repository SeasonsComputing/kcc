#include <inc/core/Core.h>
#include "MyComponentImplEx.h"

#define KCC_FILE "MyComponentImplEx"

MyComponentImplEx::MyComponentImplEx()  { kcc::Log::Scope scope(KCC_FILE, "MyComponentImplEx");  kcc::Log::info1("ctor"); }
MyComponentImplEx::~MyComponentImplEx() { kcc::Log::Scope scope(KCC_FILE, "~MyComponentImplEx"); kcc::Log::info1("dtor"); }
void MyComponentImplEx::go(const Data& d)  
{ 
    kcc::Log::Scope scope(KCC_FILE, "go");
    kcc::Log::info1("data [%s]", d.name.c_str());

    kcc::Log::info1("testing NESTED simple component");
    kcc::AutoPtr<IMyComponent> s(KCC_COMPONENT(IMyComponent, "k_simplecomponent"));
    s->go(IMyComponent::Data("simple NESTED test"));
}

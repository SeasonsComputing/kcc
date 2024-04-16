#include <inc/core/Core.h>
#include "../IMyComponent.h"

#define KCC_FILE "SimpleComponentImpl"

struct SimpleComponentImpl : IMyComponent 
{
    SimpleComponentImpl()  { kcc::Log::Scope scope(KCC_FILE, "SimpleComponentImpl");  kcc::Log::info1("ctor"); }
    ~SimpleComponentImpl() { kcc::Log::Scope scope(KCC_FILE, "~SimpleComponentImpl"); kcc::Log::info1("dtor"); }
    void go(const Data& d) { kcc::Log::Scope scope(KCC_FILE, "go");                   kcc::Log::info1("data [%s]", d.name.c_str()); }
};

KCC_COMPONENT_FACTORY_IMPL(SimpleComponentImpl);

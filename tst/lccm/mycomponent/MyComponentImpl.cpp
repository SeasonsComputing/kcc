#include <inc/core/Core.h>
#include "MyComponentImpl.h"

#define KCC_FILE "MyComponentImpl"

MyComponentImpl::MyComponentImpl()      { kcc::Log::Scope scope(KCC_FILE, "MyComponentImpl");  kcc::Log::info1("ctor"); }
MyComponentImpl::~MyComponentImpl()     { kcc::Log::Scope scope(KCC_FILE, "~MyComponentImpl"); kcc::Log::info1("dtor"); }
void MyComponentImpl::go(const Data& d) { kcc::Log::Scope scope(KCC_FILE, "go"); kcc::Log::info1("data [%s]", d.name.c_str()); }

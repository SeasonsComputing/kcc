#ifndef IMyComponent_h
#define IMyComponent_h

interface IMyComponent : kcc::IComponent 
{
    struct Data 
    {
        Data(const kcc::String& n) : name(n) {}
        kcc::String name;
    };
    virtual void go(const Data& d) = 0;
};

#endif // IMyComponent_h

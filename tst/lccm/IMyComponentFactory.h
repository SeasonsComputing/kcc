#ifndef IMyComponentFactory_h
#define IMyComponentFactory_h

interface IMyComponent;

interface IMyComponentFactory : kcc::IComponentFactory 
{
    virtual IMyComponent* constructMy() = 0; // type-safe version of construct()
    virtual IMyComponent* constructMyEx() = 0;
};

#endif // IMyComponentFactory_h

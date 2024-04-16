/*
 * Kuumba C++ Core
 *
 * $Id: $
 */
#ifndef ComponentFactory_h
#define ComponentFactory_h

namespace kcc
{
    /**
     * Stock component factory implementation.
     * @param _C component implementation class for factory to create.
     *
     * @author Ted V. Kremer
     */
    template<class _C> class ComponentFactoryImpl : public IComponentFactory
    {
    public:
        IComponent* construct() { return new _C; }
    };

    /** Component factory entry point signature */
    typedef IComponentFactory* (*ComponentFactoryConstruct)();
    
    /** Component module metadata entry point signatire */
    typedef void (*ComponentModuleMetadata)(Properties&);
}

// Exported component-factory-creator entry point method name should not be mangled
#if defined(KCC_WINDOWS)
#   define KCC_COMPONENT_FACTORY_DECL extern "C" __declspec(dllexport)
#else
#   define KCC_COMPONENT_FACTORY_DECL extern "C"
#endif

// Exported entry point into component module
#define KCC_COMPONENT_FACTORY_NAME "k_factoryConstruct"

// Exported entry point accessor to component module metadata
#define KCC_COMPONENT_FACTORY_METADATA "k_factoryMetadata"

/**
 * Utility macro to create component module entry point.
 * @param _CF component factory implementation class to construct.
 */
#define KCC_COMPONENT_FACTORY_CUST(_CF) \
    KCC_COMPONENT_FACTORY_DECL kcc::IComponentFactory* k_factoryConstruct() { return new _CF; }

/**
 * Utility macro to create component module entry point and instanstiate
 * a stock single-component-construction component factory.
 * @param _C component implementation class to construct.
 */
#define KCC_COMPONENT_FACTORY_IMPL(_C) \
    typedef kcc::ComponentFactoryImpl<_C> _C##Factory; \
    KCC_COMPONENT_FACTORY_CUST(_C##Factory)

/**
 * Utility macros to create component module metadata.
 *
 * Stock Usage (stock factory):
 *
 *    KCC_COMPONENT_FACTORY_IMPL(MyComponent)
 *  
 *    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(MyComponent, IMyComponent)
 *        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, "$Id: ... $")
 *    KCC_COMPONENT_FACTORY_METADATA_END
 *
 * Custom Usage (custom factory):
 *
 *    KCC_COMPONENT_FACTORY_CUST(MyComponentFactory)
 *   
 *    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(MyComponentFactory, IMyComponentFactory, MyComponent, IMyComponent)
 *        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, "$Id: ... $")
 *    KCC_COMPONENT_FACTORY_METADATA_END
 */

/**
 * ComponentFactory metadata keys
 */
#define KCC_COMPONENT_FACTORY_FACTORY    "kcc.factory"
#define KCC_COMPONENT_FACTORY_IFACTORY   "kcc.ifactory"
#define KCC_COMPONENT_FACTORY_COMPONENT  "kcc.component"
#define KCC_COMPONENT_FACTORY_ICOMPONENT "kcc.icomponent"
#define KCC_COMPONENT_FACTORY_SCM        "kcc.scm"
#define KCC_COMPONENT_FACTORY_VERSION    "kcc.version"

#define KCC_COMPONENT_FACTORY_METADATA_PROPERTY(_K, _V) md.set(_K, _V);

#define KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(_F, _IF, _C, _IC) \
    KCC_COMPONENT_FACTORY_DECL void k_factoryMetadata(kcc::Properties& md) { \
        _F* pf = NULL; _IF* pif = NULL; _C* pc = NULL; _IC* pic = NULL; \
        (void)pf; (void)pif; (void)pc; (void)pic; \
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_FACTORY,    #_F) \
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_IFACTORY,   #_IF) \
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_COMPONENT,  #_C) \
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_ICOMPONENT, #_IC)

#define KCC_COMPONENT_FACTORY_METADATA_END } 

#define KCC_COMPONENT_FACTORY_METADATA_BEGIN \
    KCC_COMPONENT_FACTORY_DECL void k_factoryMetadata(kcc::Properties& md) {

#define KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(_C, _IC) \
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_CUSTOM(_C##Factory, kcc::IComponentFactory, _C, _IC)

#endif // ComponentFactory_h

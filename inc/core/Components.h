/*
 * Kuumba C++ Core
 *
 * $Id: Components.h 21993 2008-02-05 18:55:20Z tvk $
 */
#ifndef Components_h
#define Components_h

namespace kcc
{
    /**
     * Utility class to locate, bind, & manage a component module and access factory instances.
     *
     * Component Location:
     *   Components are located according to the following rules:
     *   
     *    - System Locator (default)
     *      o Absolute Path (default)
     *          The system locator will use "./" to locate a a component relative to the kcc.systemPath path
     *          To override the default relative path set 
     *            kcc.componentsSystemPathOverride={path} {default=./}
     *      o Operating System Path
     *          To have the system _not_ specify a relative path (load:"comp.dll/so") set
     *            kcc.componentsSystemPathBinding=1 (default=0)
     *
     *    - Override System Locator
     *        To override the system locator specify:
     *          kcc.componentsLocator.SystemLocator={IComponentLocator provider name}
     *          kcc.componentsLocator.{IComponentLocator provider name}=DefaultSystemLocator
     *
     *    - Per Component Locator
     *        To override a locator per component specify:
     *          kcc.componentsLocator.{component provider name}={IComponentLocator provider name}
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Components
    {
    public:
        /**
         * Accessor to component factory for component id. 
         * NOTE: to create a new factory instance use module(id)->constructFactory().
         * @param id id of component
         * @return factory instance for id (ownership NOT consumed)
         * @throw ComponentModule::ComponentNotFound component not found
         * @throw ComponentModule::FactoryNotFound factory method not found
         */
        static IComponentFactory* factory(const String& id)
            throw (ComponentModule::ComponentNotFound, ComponentModule::FactoryNotFound);

        /**
         * Accessor to component binding in cache. If not bound will create and cache binding
         * @param id id of component
         * @return module component module for id
         * @throw ComponentModule::ComponentNotFound component not found
         */
        static ComponentModule& module(const String& id) throw (ComponentModule::ComponentNotFound);

        /**
         * Accessor to cached component id's.
         * @param ids where to place the names of cached component id's
         */
        static void moduleIds(StringVector& ids);

        /**
         * Accessor to system locator
         * NOTE: to get the locator for a specific component use module definition (@see ComponentModule::locator())
         * @param defaultSystem get the default system locator otheriwse will return the overloaded locator (if overloaded)
         * @return system locator (ownership NOT consumed)
         */
        static IComponentLocator* locator(bool defaultSystem = false);

        /**
         * Accessor to system manager
         * @return system manager (ownership NOT consumed)
         */
        static IComponentModuleManager* manager();

    private:
        Components();
    };

/**
 * RTTI versions of construct and downcast (throws exceptions on bad casts)
 */
#if !defined(KCC_NO_RTTI)
    template<class _I> inline _I* Components_constructSafe(IComponentFactory* f)
        throw (ComponentModule::ComponentNotFound)
    {
        IComponent* c = f->construct();
        _I* t = dynamic_cast<_I*>(c);
        if (c != NULL && t == NULL) 
        {
            const char* component = typeid(_I).name();
            const char* actual    = typeid(*c).name();
            delete c; // ownership consumed so clean bad component type
            throw ComponentModule::ComponentNotFound(Strings::printf("invalid cast: component=[%s] actual=[%s]", component, actual));
        }
        return t;
    }
    template<class _I> inline _I* Components_downcastSafe(IComponent* c)
        throw (ComponentModule::ComponentNotFound)
    {
        _I* t = dynamic_cast<_I*>(c);
        if (c != NULL && t == NULL) 
        {
            const char* component = typeid(_I).name();
            const char* actual    = typeid(*c).name();
            throw ComponentModule::ComponentNotFound(Strings::printf("invalid cast: component=[%s] actual=[%s]", component, actual));
        }
        return t;
    }
#endif
}

/**
 * Utility macros to access a component factory, downcast a component, or construct a component instance.
 *
 *   KCC_FACTORY(_I, _C)            - Returns the component factory instance for _C of type _I (ownership NOT consumed)
 *   KCC_FACTORY_CONSTRUCT(_I, _F)  - Constructs a component _I using the factory instance _F (ownership IS consumed)
 *   KCC_COMPONENT(_I, _C)          - Constructs a component instance for _C of type _I (ownership IS consumed)
 *   KCC_COMPONENT_DOWNCAST(_I, _O) - Downcasts component instance _O to a derived instance _I
 */
#if !defined(KCC_NO_RTTI)
#   define KCC_FACTORY(_I, _C)            (kcc::Components_downcastSafe<_I>(kcc::Components::factory(_C)))
#   define KCC_FACTORY_CONSTRUCT(_I, _F)  (kcc::Components_downcastSafe<_I>((_F)->construct()))
#   define KCC_COMPONENT_DOWNCAST(_I, _O) (kcc::Components_downcastSafe<_I>(_O))
#   define KCC_COMPONENT(_I, _C)          (kcc::Components_constructSafe<_I>(kcc::Components::factory(_C)))
#else
#   define KCC_FACTORY(_I, _C)            ((_I*)kcc::Components::factory(_C))
#   define KCC_FACTORY_CONSTRUCT(_I, _F)  ((_I*)(_F)->construct())
#   define KCC_COMPONENT_DOWNCAST(_I, _O) ((_I*)(_O))
#   define KCC_COMPONENT(_I, _C)          ((_I*)kcc::Components::factory(_C)->construct())
#endif

/**
 * Components configurations
 */
#define KCC_COMPONENTS_SYSPATHBINDING   "kcc.componentsSystemPathBinding"
#define KCC_COMPONENTS_SYSPATHOVERRIDE  "kcc.componentsSystemPathOverride"
#define KCC_COMPONENTS_LOCATOR          "kcc.componentsLocator."


#endif // Components_h

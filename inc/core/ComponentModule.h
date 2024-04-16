/*
 * Kuumba C++ Core
 *
 * $Id: ComponentModule.h 20964 2007-10-06 19:52:51Z tvk $
 */
#ifndef ComponentModule_h
#define ComponentModule_h

namespace kcc
{
    interface IComponentLocator;
    interface IComponentModuleManager;

    /**
     * Provides the binding to a component module and locating
     * the factory construction entry point so the component factory may
     * be created.
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT ComponentModule
    {
    public:
        /** Component not found exception */
        KCC_COMPONENT_EXCEPTION(ComponentNotFound);

        /** Factory not found exception */
        KCC_COMPONENT_EXCEPTION(FactoryNotFound);

        /** Component definition */
        struct ComponentDefinition
        {
            ComponentDefinition(const String& _id) : id(_id) {}
            String     id;
            String     name;
            String     path;
            Properties metadata;
        };

        /**
         * Construct a module instance using location service
         * @param id id of component
         * @param locator locator provider (required) (ownership NOT consumed)
         * @param manager module manager (optional) (ownership NOT consumed)
         */
        ComponentModule(const String& id, IComponentLocator* locator, IComponentModuleManager* manager = NULL);

        /** Clean-up component module (will onload if loaded) */
        ~ComponentModule();

        /**
         * Bind to module (load object library).
         * Calling more then once will not reload. To reload unbind then bind again.
         * @throw ComponentNotFound component not found
         */
        void bind() throw (ComponentNotFound);

        /** Unbind from module (unload object library) */
        void unbind();

        /**
         * Query if component module is bound
         * @return true if bound
         */
        bool bound();

        /**
         * Construct factory for component (will bind to module if not bound)
         * @return factory instance for component module (owner IS consumed)
         * @throw ComponentNotFound component not found
         * @throw FactoryNotFound factory method not found
         */
        IComponentFactory* constructFactory() throw (ComponentNotFound, FactoryNotFound);

        /** Accessors */
        inline const ComponentDefinition&     definition() const { return m_definition; }
        inline const IComponentLocator*       locator()    const { return m_locator; }
        inline const IComponentModuleManager* manager()    const { return m_manager; }

    private:
        // Attributes
        ComponentDefinition      m_definition;
        IComponentLocator*       m_locator;
        IComponentModuleManager* m_manager;
        Platform::Module         m_module;
    };
    
    /**
     * Provider for locating a component module
     *
     * @author Ted V. Kremer
     */
    interface IComponentLocator : IComponent
    {
        /**
         * Locate definition
         * @param definition definition of component
         * @throw ComponentModule::ComponentNotFound component not found
         */
        virtual void locate(ComponentModule::ComponentDefinition& definition) throw (ComponentModule::ComponentNotFound) = 0;
    };

    /**
     * Interface for managing component binding
     *
     * @author Ted V. Kremer
     */
    interface IComponentModuleManager : IComponent
    {
        /**
         * Notification of binding (called post binding)
         * @param module component module bound
         */
        virtual void onBind(ComponentModule* module) = 0;

        /**
         * Notification of unbinding (called prior to unbinding)
         * @param module component module bound
         */
        virtual void onUnbind(ComponentModule* module) = 0;
    };
}

#endif // ComponentModule_h

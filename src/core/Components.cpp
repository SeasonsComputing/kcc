/*
 * Kuumba C++ Core
 *
 * $Id: Components.cpp 21993 2008-02-05 18:55:20Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "Components"

namespace kcc
{
    // Properties
    static const String k_keySystemPathBinding    (KCC_COMPONENTS_SYSPATHBINDING);
    static const String k_keySystemPathOverride   (KCC_COMPONENTS_SYSPATHOVERRIDE);
    static const String k_keyModuleLocator        (KCC_COMPONENTS_LOCATOR);
    static const String k_defModuleLocator        ("SystemLocator");
    static const String k_defSystemLocator        ("DefaultSystemLocator");
    static const long   k_defSystemPathBinding    = KCC_PROPERTY_FALSE;
    static const String k_defSystemPathOverride   ("./");
    static const String k_keyModuleLocatorOverride(k_keyModuleLocator + k_defModuleLocator);

    // Helper class to cache modules
    struct ComponentsModuleState : Core::ModuleState, IComponentModuleManager, IComponentLocator
    {
        // Attributes
        typedef std::map<String, IComponentLocator*> ComponentLocators;
        typedef std::map<String, ComponentModule*>   ComponentModules;
        typedef std::map<String, IComponentFactory*> ComponentFactories;
        ComponentLocators  m_locators;
        ComponentModules   m_modules;
        ComponentFactories m_factories;
        Mutex              m_sentinel;
        ~ComponentsModuleState()
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "ComponentsModuleState::~ComponentsModuleState");
            for (ComponentModules::iterator m = m_modules.begin(); m != m_modules.end(); m++)
                delete m->second;
            for (ComponentLocators::iterator l = m_locators.begin(); l != m_locators.end(); l++)
                delete l->second;
            if (!m_factories.empty()) Log::warning("factory leaks detected");
        }
        
        // locator: system locator
        IComponentLocator* locator(bool defaultSystem) 
        {
            Mutex::Lock lock(m_sentinel);
            if (defaultSystem) return this;
            ComponentLocators::iterator l = m_locators.find(k_defModuleLocator);
            return l == m_locators.end() ? this : l->second;
        }
        
        // manager: system manager
        IComponentModuleManager* manager() { return this; }

        // locate: system locator
        void locate(ComponentModule::ComponentDefinition& definition) throw (ComponentModule::ComponentNotFound)
        {
            definition.name = Platform::moduleSystemName(definition.id);
            if (Core::properties().get(k_keySystemPathBinding, k_defSystemPathBinding) == KCC_PROPERTY_TRUE)
                definition.path.clear();
            else
                definition.path = Core::properties().get(k_keySystemPathOverride, k_defSystemPathOverride);
        }

        // onBind: register module for management
        void onBind(ComponentModule* module) 
        {
            Mutex::Lock lock(m_sentinel);
            if (module->manager() != this) m_modules[module->definition().id] = module;
        }

        // onUnbind: remove factory when unbound
        void onUnbind(ComponentModule* module)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "ComponentsModuleState::onUnbind");
            ComponentFactories::iterator i = m_factories.find(module->definition().id);
            if (i != m_factories.end()) 
            {
                delete i->second;
                m_factories.erase(i);
            }
            else
            {
                Log::warning("module not managed: id=[%]", module->definition().id.c_str());
            }
        }

        // module: accessor to module (lazy creation & cached)
        ComponentModule& module(const String& id)
            throw (ComponentModule::FactoryNotFound)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "ComponentsModuleState::module");
            ComponentModule* m = NULL;
            ComponentModules::iterator mi = m_modules.find(id);
            if (mi != m_modules.end()) m = mi->second;
            else
            {
                // locator
                IComponentLocator* l = NULL;
                String locatorComponent = Core::properties().get(k_keyModuleLocator + id, k_defModuleLocator);
                if (locatorComponent == k_defModuleLocator)
                {
                    // system locator
                
                    // override?
                    const String& override = Core::properties().get(k_keyModuleLocatorOverride, Strings::empty());
                    if (!override.empty() && m_locators.find(k_defModuleLocator) == m_locators.end()) 
                    {
                        l = KCC_COMPONENT(IComponentLocator, override);
                        m_locators[k_defModuleLocator] = l;
                    }
                    else
                    {
                        l = locator(false);
                    }
                }
                else if (locatorComponent == k_defSystemLocator)
                {
                    // default system locator
                    l = locator(true);
                }
                else
                {
                    // per component locator
                    ComponentLocators::iterator li = m_locators.find(locatorComponent);
                    if (li != m_locators.end()) l = li->second;
                    else
                    {
                        l = KCC_COMPONENT(IComponentLocator, locatorComponent);
                        m_locators[locatorComponent] = l;
                    }
                    if (l == NULL) throw ComponentModule::FactoryNotFound("component module locator not found: " + id);
                }
                
                // module
                m = new ComponentModule(id, l, manager());
                m_modules[id] = m;
            }
            return *m;
        }

        // factory: accessor to factor (lazy creation & cached)
        IComponentFactory* factory(const String& id)
            throw (ComponentModule::ComponentNotFound, ComponentModule::FactoryNotFound)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "ComponentsModuleState::factory");
            IComponentFactory* f = NULL;
            ComponentFactories::iterator i = m_factories.find(id);
            if (i != m_factories.end()) f = i->second;
            else
            {
                f = module(id).constructFactory();
                m_factories[id] = f;
            }
            return f;
        }

        // moduleIds: fetch collection of module id's
        void moduleIds(StringVector& ids)
        {
            Mutex::Lock lock(m_sentinel);
            Log::Scope scope(KCC_FILE, "ComponentsModuleState::moduleIds");
            ids.clear();
            ids.reserve(m_modules.size());
            for (ComponentModules::iterator i = m_modules.begin(); i != m_modules.end(); i++)
                ids.push_back(i->first);
        }
    };

    //
    // Components Implementation
    //

    // constructFactory: construct factory for component id
    IComponentFactory* Components::factory(const String& componentId)
        throw (ComponentModule::ComponentNotFound, ComponentModule::FactoryNotFound)
    {
        return KCC_STATE(ComponentsModuleState).factory(componentId);
    }

    // module: accessor to module
    ComponentModule& Components::module(const String& componentId)
        throw (ComponentModule::ComponentNotFound)
    {
        return KCC_STATE(ComponentsModuleState).module(componentId);
    }

    // moduleIds: fetch collection of module id's
    void Components::moduleIds(StringVector& ids)
    {
        return KCC_STATE(ComponentsModuleState).moduleIds(ids);
    }
    
    // locator: accessor
    IComponentLocator* Components::locator(bool defaultSystem)
    {
        return KCC_STATE(ComponentsModuleState).locator(defaultSystem);
    }

    // manager: accessor
    IComponentModuleManager* Components::manager()
    {
        return KCC_STATE(ComponentsModuleState).manager();
    }
}

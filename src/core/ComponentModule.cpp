/*
 * Kuumba C++ Core
 *
 * $Id: ComponentModule.cpp 20964 2007-10-06 19:52:51Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "ComponentModule"

namespace kcc
{
    // Properties
    static const String k_defVersion("0");
    static const kcc::String k_mdRequired[] =
    {
        KCC_COMPONENT_FACTORY_FACTORY,
        KCC_COMPONENT_FACTORY_IFACTORY,
        KCC_COMPONENT_FACTORY_COMPONENT,
        KCC_COMPONENT_FACTORY_ICOMPONENT,
        KCC_COMPONENT_FACTORY_SCM,
        KCC_COMPONENT_FACTORY_VERSION,
        kcc::Strings::empty()
    };

    // ctor/dtor
    ComponentModule::ComponentModule(const String& id, IComponentLocator* locator, IComponentModuleManager* manager) : 
        m_definition(id), m_locator(locator), m_manager(manager), m_module(NULL)
    {}
    ComponentModule::~ComponentModule() { unbind(); }

    // bind: bind to component
    void ComponentModule::bind() throw (ComponentNotFound)
    {
        Log::Scope scope(KCC_FILE, "bind");
        if (m_module != NULL) return; // already bound

        // locate & bind
        m_locator->locate(m_definition);
        m_module = Platform::moduleBind(m_definition.name, m_definition.path);
        if (m_module == NULL) throw ComponentNotFound(m_definition.id);

        // metadata
        void* md = Platform::moduleFunction(m_module, KCC_COMPONENT_FACTORY_METADATA, false);
        if (md != NULL) (ComponentModuleMetadata(md))(m_definition.metadata);
        // medata version: imbue default when absent
        if (!m_definition.metadata.exists(KCC_COMPONENT_FACTORY_VERSION)) 
            m_definition.metadata.set(KCC_COMPONENT_FACTORY_VERSION, k_defVersion);
        kcc::String mdlog;
        mdlog.reserve(1024);
        for (int i = 0; !k_mdRequired[i].empty(); i++)
        {
            const kcc::String& k = k_mdRequired[i];
            const kcc::String& v = m_definition.metadata.get(k, kcc::Strings::empty());
            if (v.empty()) 
            {
                kcc::Log::warning(
                    "missing required component metadata: id=[%s] md=[%s]", 
                    m_definition.id.c_str(),
                    k.c_str());
            }
            if (!mdlog.empty()) mdlog += ", ";
            mdlog += k + "=>" + v;
        } 
        if (m_manager != NULL) m_manager->onBind(this);
        Log::info1(
            "module bind: id=[%s] name=[%s] path=[%s] md=[%s]", 
            m_definition.id.c_str(), 
            m_definition.name.c_str(),
            m_definition.path.c_str(),
            mdlog.c_str());
    }

    // unbind: unbind component
    void ComponentModule::unbind()
    {
        Log::Scope scope(KCC_FILE, "unbind");
        if (m_module == NULL) return;  // not bound
        if (m_manager != NULL) m_manager->onUnbind(this);
        Platform::moduleUnbind(m_module);
        m_module = NULL;
        Log::info1("module unbind: id=[%s]", m_definition.id.c_str());
    }

    // bound: query if bound
    bool ComponentModule::bound() { return m_module != NULL; }

    // constructFactory: locate factory creation entry point and call
    IComponentFactory* ComponentModule::constructFactory()
        throw (ComponentNotFound, FactoryNotFound)
    {
        Log::Scope scope(KCC_FILE, "constructFactory");
        bind();
        void* ctor = Platform::moduleFunction(m_module, KCC_COMPONENT_FACTORY_NAME);
        if (ctor == NULL) throw FactoryNotFound(m_definition.id);
        return (ComponentFactoryConstruct(ctor))();
    }
}

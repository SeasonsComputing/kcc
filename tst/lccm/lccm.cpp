#include <inc/core/Core.h>
#include "IMyComponent.h"
#include "IMyComponentFactory.h"

#define KCC_FILE    "lccm"
#define KCC_VERSION "$Id: lccm.cpp 15199 2007-03-09 17:57:17Z tvk $"

int main(int argc, const char* argv[])
{
    std::cout << "lccm - Kuumba Test component system." << std::endl;

    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       1L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    kcc::Log::out("begin lccm testing");
    try
    {
        kcc::Log::out("testing simple component");
        kcc::AutoPtr<IMyComponent> s(KCC_COMPONENT(IMyComponent, "k_simplecomponent"));
        s->go(IMyComponent::Data("simple test"));

        kcc::Log::out("testing stock component loading");
        kcc::AutoPtr<IMyComponent> c(KCC_COMPONENT(IMyComponent, "k_mycomponent"));
        c->go(IMyComponent::Data("stock test"));

        kcc::Log::out("testing custom component factory loading");
        IMyComponentFactory* f = KCC_FACTORY(IMyComponentFactory, "k_mycomponent");

        kcc::AutoPtr<IMyComponent> c1(f->constructMy());
        c1->go(IMyComponent::Data("test"));

        kcc::AutoPtr<IMyComponent> c2(f->constructMyEx());
        c2->go(IMyComponent::Data("testEx"));

        kcc::StringVector ids;
        kcc::Components::moduleIds(ids);
        for (
            kcc::StringVector::iterator i = ids.begin();
            i != ids.end();
            i++)
        {
            kcc::ComponentModule& m = kcc::Components::module(*i);
            const kcc::ComponentModule::ComponentDefinition& c = m.definition();
            const kcc::Char* path = c.path.empty() ? "{system}" : c.path.c_str();
            kcc::Log::out(
                "module status: id=[%s] path=[%s] name=[%s] bound=[%s]", 
                c.id.c_str(),
                path,
                c.name.c_str(),
                (m.bound() ? "yes" : "no"));
        }

        kcc::Log::out("completed lccm testing");
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}

/*
 * Kuumba C++ Core
 *
 * $Id: IComponent.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef IComponent_h
#define IComponent_h

namespace kcc
{
    /**
     * Component type specifier. All components must derive from this interface.
     * Compoment sub-types must NOT have data members nor inline methods and MUST
     * have all methods declared as pure-virtual (= 0).
     *
     * IComponent ptr's are downcast to sub-type's (static_cast<>). Compiling
     * with RTTI allows dynamic_cast<> to be used which may then be used to
     * distinguish sub-types.
     *
     * @author Ted V. Kremer
     */
    interface IComponent
    {
        /**
         * All components must be heap based and have a v-table. Addtionally the
         * dtor must be the first entry in the v-table, satisfied by the following
         * implementation.
         */
        virtual ~IComponent() {};
    };

    /**
     * Component factory. All component modules must have only 1 factory.
     * Factory's should be specialized to add additional construction param's or
     * utility methods or to create other components from this module.
     *
     * @author Ted V. Kremer
     */
    interface IComponentFactory : IComponent
    {
        /**
         * Factory method to construct a component. All component implementations
         * must have at least an empty constructor for creation. Ownership
         * of component is consumed by caller.
         */
        virtual IComponent* construct() = 0;
    };
}
    
/**
 * Utility macro to create component exceptions.
 * Required to force some compilers to emit exception code for 
 * stateless/methodless type-only exceptions
 *
 * Stock Usage:
 *    KCC_COMPONENT_EXCEPTION(MyCompException)
 *
 * Custom Usage:
 *    KCC_COMPONENT_EXCEPTION_BEGIN(MyCustCompException)
 *        String target;
 *        String regex;
 *        MyCustCompException(const String& what, const String& _target, const String& _regex) 
 *            : Exception(what), target(_target), regex(_regex)
 *        {}
 *    KCC_COMPONENT_EXCEPTION_END
 */
#if defined(KCC_WINDOWS)
#   define KCC_COMPONENT_EXCEPTION_BEGIN(_C) \
    struct _C : kcc::Exception \
    { \
        _C(const kcc::String& msg) throw () : kcc::Exception(msg) {} \
        virtual ~_C() throw () {}
#elif defined(KCC_LINUX)
#   define KCC_COMPONENT_EXCEPTION_BEGIN(_C) \
    struct _C : kcc::Exception \
    { \
        _C(const kcc::String& msg) throw () : kcc::Exception(msg) {} \
        virtual ~_C() throw () {} \
        void _emit() __attribute__ ((used)) { throw _C("emit"); }
#endif
#define KCC_COMPONENT_EXCEPTION_END };
#define KCC_COMPONENT_EXCEPTION(_C) \
    KCC_COMPONENT_EXCEPTION_BEGIN(_C) \
    KCC_COMPONENT_EXCEPTION_END

#endif // IComponent_h

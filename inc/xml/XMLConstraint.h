/**
 * GENERATED FILE: 2006-04-28
 */
#ifndef XMLConstraint_h
#define XMLConstraint_h

namespace kcc
{
    struct XMLConstraint
    {
        static const String& required()
        {
            static const String k_required("required");
            return k_required;
        }
        static const String& optional()
        {
            static const String k_optional("optional");
            return k_optional;
        }

    private:
        XMLConstraint();
    };
}

#endif // XMLConstraint_h

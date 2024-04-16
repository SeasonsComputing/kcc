/**
 * GENERATED FILE: 2006-04-28
 */
#ifndef XMLValue_h
#define XMLValue_h

namespace kcc
{
    struct XMLValue
    {
        static const String& boolean()
        {
            static const String k_boolean("{boolean}");
            return k_boolean;
        }
        static const String& number()
        {
            static const String k_number("{number}");
            return k_number;
        }
        static const String& isodate()
        {
            static const String k_isodate("{isodate}");
            return k_isodate;
        }

    private:
        XMLValue();
    };
}

#endif // XMLValue_h

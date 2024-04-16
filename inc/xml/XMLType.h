/**
 * GENERATED FILE: 2006-04-28
 */
#ifndef XMLType_h
#define XMLType_h

namespace kcc
{
    struct XMLType
    {
        static const String& value()
        {
            static const String k_value("value");
            return k_value;
        }
        static const String& atomic()
        {
            static const String k_atomic("atomic");
            return k_atomic;
        }
        static const String& list()
        {
            static const String k_list("list");
            return k_list;
        }
        static const String& cdata()
        {
            static const String k_cdata("cdata");
            return k_cdata;
        }

    private:
        XMLType();
    };
}

#endif // XMLType_h

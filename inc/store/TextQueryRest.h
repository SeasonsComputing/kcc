/**
 * GENERATED FILE: 2007-04-24
 */
#ifndef TextQueryRest_h
#define TextQueryRest_h

namespace kcc
{
    struct TextQueryRest
    {
        static const String& query()
        {
            static const String k_query("/query");
            return k_query;
        }
        static const String& queryExpression()
        {
            static const String k_queryExpression("expr");
            return k_queryExpression;
        }
        static const String& queryFlush()
        {
            static const String k_queryFlush("flush");
            return k_queryFlush;
        }
        static const String& queryContents()
        {
            static const String k_queryContents("contents");
            return k_queryContents;
        }
        static const String& queryMax()
        {
            static const String k_queryMax("max");
            return k_queryMax;
        }
        static const String& queryId()
        {
            static const String k_queryId("id");
            return k_queryId;
        }
        static const String& queryRow()
        {
            static const String k_queryRow("row");
            return k_queryRow;
        }
        static const String& status()
        {
            static const String k_status("/status");
            return k_status;
        }
        static const String& statusDetail()
        {
            static const String k_statusDetail("detail");
            return k_statusDetail;
        }
        static const String& close()
        {
            static const String k_close("/close");
            return k_close;
        }
        static const String& shutdown()
        {
            static const String k_shutdown("/shutdown");
            return k_shutdown;
        }
        static const String& service()
        {
            static const String k_service("/service");
            return k_service;
        }

    private:
        TextQueryRest();
    };
}

#endif // TextQueryRest_h

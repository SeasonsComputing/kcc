/*
 * Kuumba C++ Core
 *
 * $Id: MySql.cpp 21697 2007-12-07 17:19:50Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/store/ISQL.h>

#if defined(KCC_WINDOWS)
#   include "windows.h"
#endif

namespace odbc
{
    #include "sqlext.h"
}

#define KCC_FILE    "OdbcSql"
#define KCC_VERSION "$Id: $"

namespace kcc
{
    // Configuration 

    // Constants

    // OdbcSql field implementation

    // OdbcSql statement implementation

    // OdbcSql result set implementation

    // OdbcSql query implementation

    // OdbcSql update implementation

    // OdbcSql connection implementation

    // OdbcSql connection manager
    struct OdbcSql : ISQL
    {
        // Attributes
        odbc::HENV m_env;
        OdbcSql() : m_env(NULL) {}
        ~OdbcSql() {}

        // init: get default params
        bool init(const Properties& config) 
        {
            Log::Scope scope(KCC_FILE, "init");
            return true; 
        }

        // connect: connect using init params
        ISQLConnection* connect() throw (SQLException) { return NULL; }
    };
    
    //
    // MySql factory
    //

    KCC_COMPONENT_FACTORY_IMPL(OdbcSql)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(OdbcSql, ISQL)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}

/*
 * Kuumba C++ Core
 *
 * $Id: MySql.cpp 22625 2008-03-09 22:51:49Z tvk $
 */
#include <inc/core/Core.h>
#include <inc/store/ISQL.h>

namespace mysql
{
    #include "my_global.h"
    #include "mysql.h"
    #undef bool // fix mysql bool macro
};

#define KCC_FILE    "MySql"
#define KCC_VERSION "$Id: MySql.cpp 22625 2008-03-09 22:51:49Z tvk $"

namespace kcc
{
    // Configuration 
    static const String k_keyHost    ("MySql.host");
    static const String k_keyPort    ("MySql.port");
    static const String k_keyDB      ("MySql.db");
    static const String k_keyUser    ("MySql.user");
    static const String k_keyPassword("MySql.password");
    static const long   k_defPort    = 3306L;

    // Constants
    static const int k_sz = 128; // default string siz

    // MySql field implementation
    struct MySqlField : ISQLField
    {
        Type m_type;
        bool m_rebind;
        bool m_null;
        bool m_truncated;
        long m_length;
        union Field
        {
            short             sht;
            long              lng;
            long long         lnglng;
            float             flt;
            double            dbl;
            char              byt;
            char*             str;
            mysql::MYSQL_TIME tim;            
        } m_v;
        mysql::MYSQL_BIND* m_bind;
        MySqlField(Type t) : m_type(t) { init(); }
        ~MySqlField() { clear(); }

        // init: initialize field
        void init()
        {
            m_rebind    = true;
            m_null      = true;
            m_truncated = false;
            m_length    = 0L;
            m_bind      = NULL;
            std::memset(&m_v, 0, sizeof(m_v));
        }

        // clear: clear memory used by field
        void clear()
        {
            if (m_type == T_STRING && m_v.str != NULL) delete [] m_v.str;
            init();
        }

        // Utility
        Type type()       { return m_type; }
        bool truncated()  { return m_truncated; }
        bool null()       { return m_null; }
        void null(bool n) { m_null = n; }

        // toString: convert to string
        String toString()
        {
            if (m_null) return "(null)";
            switch (m_type)
            {
            case T_SHORT:    return Strings::printf("%d",   m_v.sht);
            case T_LONG:     return Strings::printf("%ld",  m_v.lng);
            case T_LONGLONG: return Strings::printf("%lld", m_v.lnglng);
            case T_BYTE:     return Strings::printf("%d",   m_v.byt);
            case T_FLOAT:    return Strings::printf("%f",   m_v.flt);
            case T_DOUBLE:   return Strings::printf("%f",   m_v.dbl);
            case T_STRING:   return m_v.str == NULL ? Strings::empty() : String(m_v.str);
            case T_DATE:     return Strings::printf("%04d-%02d-%02d", m_v.tim.year, m_v.tim.month, m_v.tim.day);
            case T_TIME:     return Strings::printf("%02d:%02d:%02d", m_v.tim.hour, m_v.tim.minute, m_v.tim.second);
            case T_DATETIME: 
                return Strings::printf(
                    "%04d-%02d-%02d %02d:%02d:%02d", 
                    m_v.tim.year, m_v.tim.month, m_v.tim.day, 
                    m_v.tim.hour, m_v.tim.minute, m_v.tim.second);
            case T_BINARY:   return "(binary)";
            };
            return "(unknown)";
        }

        // Field Accessors
        short     fShort()    throw (SQLException) { ok(T_SHORT);    return m_v.sht; }
        long      fLong()     throw (SQLException) { ok(T_LONG);     return m_v.lng; }
        long long fLongLong() throw (SQLException) { ok(T_LONGLONG); return m_v.lnglng; }
        float     fFloat()    throw (SQLException) { ok(T_FLOAT);    return m_v.flt; }
        double    fDouble()   throw (SQLException) { ok(T_DOUBLE);   return m_v.dbl; }
        char      fByte()     throw (SQLException) { ok(T_BYTE);     return m_v.byt; }
        SQLDate   fDate()     throw (SQLException) { ok(T_DATE);     return SQLDate(m_v.tim.year, m_v.tim.month, m_v.tim.day, 0, 0, 0); }
        SQLDate   fTime()     throw (SQLException) { ok(T_TIME);     return SQLDate(0, 0, 0, m_v.tim.hour, m_v.tim.minute, m_v.tim.second); }
        SQLDate   fDateTime() throw (SQLException) { ok(T_DATETIME); return SQLDate(m_v.tim.year, m_v.tim.month, m_v.tim.day, m_v.tim.hour, m_v.tim.minute, m_v.tim.second); }
        String    fString()   throw (SQLException) { ok(T_STRING);   return m_v.str == NULL ? Strings::empty() : String(m_v.str); }

        // Field Modifiers
        void fShort(short sht)           throw (SQLException) { ok(T_SHORT);    m_null = false; m_v.sht    = sht; }
        void fLong(long lng)             throw (SQLException) { ok(T_LONG);     m_null = false; m_v.lng    = lng; }
        void fLongLong(long long ll)     throw (SQLException) { ok(T_LONGLONG); m_null = false; m_v.lnglng = ll; }
        void fFloat(float flt)           throw (SQLException) { ok(T_FLOAT);    m_null = false; m_v.flt    = flt; }
        void fDouble(double dbl)         throw (SQLException) { ok(T_DOUBLE);   m_null = false; m_v.dbl    = dbl; }
        void fByte(char byt)             throw (SQLException) { ok(T_BYTE);     m_null = false; m_v.byt    = byt; }
        void fDate(const SQLDate& d)     throw (SQLException) { ok(T_DATE);     m_null = false; date(d); }
        void fTime(const SQLDate& d)     throw (SQLException) { ok(T_TIME);     m_null = false; date(d); }
        void fDateTime(const SQLDate& d) throw (SQLException) { ok(T_DATETIME); m_null = false; date(d); }
        void fString(const String& str)  throw (SQLException) 
        { 
            ok(T_STRING); 
            m_null = false; 
            // expand string if needed
            if (m_bind->buffer_length < str.size()+1)
            {
                delete [] m_v.str;
                m_rebind              = true;
                m_v.str               = new char[str.size()+1]; // +1 for null
                m_bind->buffer        = m_v.str;
                m_bind->buffer_length = str.size();
            }
            m_length = str.size();
            std::memcpy(m_v.str, str.c_str(), str.size());
            m_v.str[str.size()] = 0;
        }
        void fBinary(char* bin, int sz) throw (SQLException) 
        { 
            ok(T_BINARY);
            m_null = false; 
            throw SQLException("not implemented");
        }

        // ok: validate type
        inline void ok(Type t) throw (SQLException) { if (m_type != t) throw SQLException("attempt to access value of wrong type"); }

        // date: assign SQLDate to MYSQL_DATE
        inline void date(const SQLDate& dt)
        {
            m_v.tim.year   = dt.year;
            m_v.tim.month  = dt.month; 
            m_v.tim.day    = dt.day;
            m_v.tim.hour   = dt.hour;
            m_v.tim.minute = dt.minute;
            m_v.tim.second = dt.second;
        }

        // rebinding
        inline bool rebind()       { return m_rebind; }
        inline void rebind(bool r) { m_rebind = r; }

        // attach: attach MySQL binding to field
        void attach(mysql::MYSQL_BIND* b) throw (SQLException)
        {
            clear();
            m_bind           = b;
            m_bind->is_null  = (mysql::my_bool*) &m_null;
            m_bind->length   = (unsigned long*) &m_length;
            m_bind->error    = (mysql::my_bool*) &m_truncated;
            switch (m_type)
            {
            case ISQLField::T_SHORT:
                b->buffer_type        = mysql::MYSQL_TYPE_SHORT;
                b->buffer             = (char *) &m_v.sht;
                break;
            case ISQLField::T_LONG:
                b->buffer_type        = mysql::MYSQL_TYPE_LONG;
                b->buffer             = (char *) &m_v.lng;
                break;
            case ISQLField::T_LONGLONG:
                b->buffer_type        = mysql::MYSQL_TYPE_LONGLONG;
                b->buffer             = (char *) &m_v.lnglng;
                break;
            case ISQLField::T_FLOAT:
                m_bind->buffer_type   = mysql::MYSQL_TYPE_FLOAT;
                m_bind->buffer        = (char *) &m_v.flt;
                break;
            case ISQLField::T_DOUBLE:
                m_bind->buffer_type   = mysql::MYSQL_TYPE_DOUBLE;
                m_bind->buffer        = (char *) &m_v.dbl;
                break;
            case ISQLField::T_BYTE:
                m_bind->buffer_type   = mysql::MYSQL_TYPE_TINY;
                m_bind->buffer        = (char *) &m_v.byt;
                break;
            case ISQLField::T_STRING:
                m_v.str               = new char[k_sz+1]; // +1 for null
                m_bind->buffer_type   = mysql::MYSQL_TYPE_STRING;
                m_bind->buffer        = m_v.str;
                m_bind->buffer_length = k_sz;
                break;
            case ISQLField::T_DATE:
            case ISQLField::T_TIME:
            case ISQLField::T_DATETIME:
                m_bind->buffer_type   = mysql::MYSQL_TYPE_TIMESTAMP;
                m_bind->buffer        = &m_v.tim;
                break;
            case ISQLField::T_BINARY:
                throw SQLException("not implemented");
            default: 
                throw SQLException("invalid field type");
            };
        }

        // update: update field with MySQL binding data
        void update(mysql::MYSQL_STMT* ps, int col) throw (SQLException)
        {
            switch (m_type)
            {
            case ISQLField::T_SHORT:
            case ISQLField::T_LONG:
            case ISQLField::T_LONGLONG:
            case ISQLField::T_FLOAT:
            case ISQLField::T_DOUBLE:
            case ISQLField::T_BYTE:
            case ISQLField::T_DATE:
            case ISQLField::T_TIME:
            case ISQLField::T_DATETIME:
                // direct binding to field
                break;
            case ISQLField::T_STRING:
                // check for additional data
                if (m_truncated)
                {
                    int   off = m_bind->buffer_length;
                    char* str = m_v.str;
                    m_v.str   = new char[m_length+1];       // +1 for null
                    std::memcpy(m_v.str, str, off);
                    delete [] str;
                    m_bind->buffer        = m_v.str+off;    // write to end of existing buf
                    m_bind->buffer_length = m_length;
                    mysql::mysql_stmt_fetch_column(ps, m_bind, col, off);
                    m_bind->buffer        = m_v.str;        // restore binding to front of buf
                    m_rebind = true;
                }
                m_v.str[m_length] = 0;                      // null terminate
                break;
            case ISQLField::T_BINARY:
                throw SQLException("not implemented");
            default: 
                throw SQLException("invalid field type");
            };
        }
    };

    // MySql statement implementation
    struct MySqlStatement
    {
        mysql::MYSQL&      m_mysql;
        mysql::MYSQL_STMT* m_ps;
        mysql::MYSQL_BIND* m_paramsBind;
        SQLFields*         m_params;
        mysql::MYSQL_BIND* m_rowBind;
        SQLFields*         m_row;
        bool               m_eor;
        bool               m_cursor;
        MySqlStatement(mysql::MYSQL& mysql) 
            : 
            m_mysql(mysql), 
            m_ps(NULL), 
            m_paramsBind(NULL), 
            m_params(NULL), 
            m_rowBind(NULL),
            m_row(NULL), 
            m_eor(true), 
            m_cursor(false)
         {}
        ~MySqlStatement() 
        {
            if (m_ps         != NULL) mysql::mysql_stmt_close(m_ps); 
            if (m_paramsBind != NULL) delete [] m_paramsBind;
            if (m_rowBind    != NULL) delete [] m_rowBind;
        }

        // init: initialize statment
        void init(const String& expression) throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::init");
            m_ps = mysql::mysql_stmt_init(&m_mysql);
            if (m_ps == NULL) throw SQLException(mysql::mysql_error(&m_mysql));
            unsigned long sz = (unsigned long) expression.length();
            if (mysql::mysql_stmt_prepare(m_ps, expression.c_str(), sz) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
        }

        // queryCursor: set cursor on or off
        void queryCursor(bool c) { m_cursor = c; }

        // queryPrepare: beqin query
        void queryPrepare(SQLFields* row, SQLFields* params) throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::queryPrepare");
            m_row    = row;
            m_params = params;
            bind();
        }

        // queryExecute: execute query
        void queryExecute() throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::queryExecute");
            bind();
            if (mysql::mysql_stmt_execute(m_ps) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
            mysql::MYSQL_RES* res = mysql::mysql_stmt_result_metadata(m_ps);
            if (res == NULL) throw SQLException(mysql::mysql_stmt_error(m_ps));
            int ef = (int) mysql::mysql_num_fields(res);
            mysql::mysql_free_result(res);
            int rf = m_row->size();
            if (rf != ef) throw SQLException(Strings::printf("invalid row binding: row fields=[%d] expression fields=[%d]", rf, ef));
            if (m_cursor && mysql::mysql_stmt_store_result(m_ps) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
            m_eor = false;
        }

        // queryRows: number of rows in results
        SQLUInt64 queryRows() { return mysql::mysql_stmt_num_rows(m_ps); }

        // querySeek: set cursor offset
        void querySeek(SQLUInt64 row) throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::querySeek");
            if (!m_cursor)          throw SQLException("cursor required to seek");
            if (row >= queryRows()) throw SQLException("seek out of range");
            mysql::mysql_stmt_data_seek(m_ps, row);
        }

        // queryFetch: fetch query results row
        bool queryFetch() throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::queryFetch");
            if (m_row == NULL) throw SQLException("invalid state: missing row to fetch into");
            if (m_eor)         throw SQLException("next called past end");

            // fetch results
            bind();
            int ret = mysql::mysql_stmt_fetch(m_ps);
            if (ret == 1) throw SQLException(mysql::mysql_stmt_error(m_ps));
            if (ret == MYSQL_NO_DATA) 
            {
                m_eor = true;
                return false;
            }
 
            // populate from binding
            for (int i = 0; i < m_row->size(); i++)
            {
                MySqlField& f = *(MySqlField*)m_row->at(i);
                f.update(m_ps, i);
            }

            return !m_eor; 
        }

        // updatePrepare: beqin update
        void updatePrepare(SQLFields* params) throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::updatePrepare");
            if (params   == NULL) throw SQLException("invalid state: params null");
            if (m_params != NULL) throw SQLException("invalid state: update prepare() already called");
            m_params = params;
            bind();
        }

        // updateExecute: execute update with new bindings
        SQLUInt64 updateExecute() throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::updateExecute");
            bind();
            if (mysql::mysql_stmt_execute(m_ps) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
            return mysql::mysql_stmt_affected_rows(m_ps);
        }

        // updateInsertId: insert id from statement update
        SQLUInt64 updateInsertId() { return mysql::mysql_stmt_insert_id(m_ps); }

        // bind: apply param and result bindings
        void bind() throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::bind");

            // attach binding if needed
            if (m_row    != NULL && m_rowBind    == NULL) attach(m_row,    m_rowBind);
            if (m_params != NULL && m_paramsBind == NULL) attach(m_params, m_paramsBind);

            // bind params
            if (m_params != NULL && rebind(m_params))
            {
                if (mysql::mysql_stmt_bind_param(m_ps, m_paramsBind) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
                int pf = (int) m_params->size();
                int ef = mysql::mysql_stmt_param_count(m_ps);
                if (pf != ef) throw SQLException(Strings::printf("invalid param binding: param fields=[%d] expression fields=[%d]", pf, ef));
            }

            // bind results
            if (m_row != NULL && rebind(m_row))
            {
                if (mysql::mysql_stmt_bind_result(m_ps, m_rowBind) > 0) throw SQLException(mysql::mysql_stmt_error(m_ps));
            }

            // reset binding status
            if (m_row    != NULL) rebind(m_row,    false);
            if (m_params != NULL) rebind(m_params, false);
        }

        // rebind: query if fields need rebinding
        bool rebind(SQLFields* fields) throw (SQLException)
        {
            bool rebind = false;
            for (int i = 0; i < fields->size(); i++)
            {
                MySqlField& f = *(MySqlField*)fields->at(i);
                if (f.rebind())
                {
                    rebind = true;
                    break;
                }
            }
            return rebind;
        }

        // rebind: modify fields to rebinding status
        void rebind(SQLFields* fields, bool rebind) throw (SQLException)
        {
            for (int i = 0; i < fields->size(); i++)
            {
                MySqlField& f = *(MySqlField*)fields->at(i);
                f.rebind(rebind);
            }
        }

        // attch: attach ISQLField to MySQL binding
        void attach(SQLFields* fields, mysql::MYSQL_BIND*& bind) throw (SQLException)
        {
            Log::Scope scope(KCC_FILE, "MySqlStatement::bindFields");
            bind = new mysql::MYSQL_BIND[fields->size()];
            std::memset(bind, 0, sizeof(mysql::MYSQL_BIND)*fields->size());
            for (int i = 0; i < fields->size(); i++)
            {
                MySqlField& f = *(MySqlField*)fields->at(i);
                f.attach(&bind[i]);
            }
        }
    };

    // MySql query implementation
    struct MySqlQuery : ISQLQuery
    {
        MySqlStatement s;
        MySqlQuery(mysql::MYSQL& mysql) : s(mysql) {}
        void      init   (const String& e, bool cursor) throw (SQLException) { s.init(e); s.queryCursor(cursor); }
        void      prepare(SQLFields& r)                 throw (SQLException) { s.queryPrepare(&r, NULL); }
        void      prepare(SQLFields& r, SQLFields& p)   throw (SQLException) { s.queryPrepare(&r, &p); }
        void      execute()                             throw (SQLException) { s.queryExecute(); }
        void      begin  (SQLFields& r)                 throw (SQLException) { prepare(r); execute(); }
        bool      next   ()                             throw (SQLException) { return s.queryFetch(); }
        SQLUInt64 rows   ()                                                  { return s.queryRows(); }
        void      seek   (SQLUInt64 r)                  throw (SQLException) { s.querySeek(r); }
    };

    // MySql update implementation
    struct MySqlUpdate : ISQLUpdate
    {
        MySqlStatement s;
        MySqlUpdate(mysql::MYSQL& mysql) : s(mysql) {}
        void      init    (const String& e) throw (SQLException) { s.init(e); }
        void      prepare (SQLFields& p)    throw (SQLException) { s.updatePrepare(&p); }
        SQLUInt64 execute ()                throw (SQLException) { return s.updateExecute(); }
        SQLUInt64 insertId()                                     { return s.updateInsertId(); }
    };

    // MySql connection implementation
    struct MySqlConnection : ISQLConnection
    {
        // Attributes
        mysql::MYSQL m_mysql;
        MySqlConnection()  { mysql::mysql_init(&m_mysql); }
        ~MySqlConnection() { mysql::mysql_close(&m_mysql); }

        // connect: connect to db
        void connect(const String& host, long port, const String& db, const String& user, const String& password) throw (SQLException)
        {
            if (mysql::mysql_real_connect(
                &m_mysql, host.c_str(), 
                user.c_str(), password.c_str(), db.c_str(), port, 
                NULL, CLIENT_MULTI_STATEMENTS) == NULL) throw SQLException(mysql::mysql_error(&m_mysql));

            // reconnect by default
            mysql::my_bool arg = true;
            if (mysql::mysql_options(&m_mysql, mysql::MYSQL_OPT_RECONNECT, (const char*)&arg) > 0)
                throw SQLException(mysql::mysql_error(&m_mysql));
        }

        // execute: execute query
        SQLUInt64 execute(const String& expression) throw (SQLException) 
        { 
            Log::Scope scope(KCC_FILE, "MySqlConnection::execute");
            if (mysql::mysql_real_query(&m_mysql, expression.c_str(), (unsigned long)expression.length()) > 0)
                throw SQLException(mysql::mysql_error(&m_mysql));
            return mysql::mysql_affected_rows(&m_mysql);
        }

        // executeBatch: execute batch query and flush
        SQLUInt64 executeBatch(const String& expression) throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::executeBatch");

            // queue batches
            SQLUInt64 rows = execute(expression);

            // write batches
            do
            {
                mysql::MYSQL_RES* res = mysql::mysql_store_result(&m_mysql);
                if (res != NULL) mysql::mysql_free_result(res);
            } while(!mysql::mysql_next_result(&m_mysql));

            return rows;
        }

        // inserId: get insert id
        SQLUInt64 insertId() { return mysql::mysql_insert_id(&m_mysql); }

        // begin: begin transaction
        void begin() throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::begin");
            if (mysql::mysql_autocommit(&m_mysql, 0) > 0) throw SQLException(mysql::mysql_error(&m_mysql));
        }

        // commit: commit transaction
        void commit() throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::commit");
            if (mysql::mysql_commit(&m_mysql) > 0) throw SQLException(mysql::mysql_error(&m_mysql));
            mysql::mysql_autocommit(&m_mysql, 1);
        }

        // rollback: rollback transaction
        bool rollback() throw ()
        {
            int err = mysql::mysql_rollback(&m_mysql);
            if (err > 0) Log::error(mysql::mysql_error(&m_mysql));
            mysql::mysql_autocommit(&m_mysql, 1);
            return err == 0;
        }

        // field: construct field
        ISQLField* field(ISQLField::Type t) throw (SQLException) { return new MySqlField(t); }

        // results: construct result set
        ISQLResultSet* results(const String& expression) throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::results");
            AutoPtr<MySqlQuery> q(new MySqlQuery(m_mysql));
            q->init(expression, false);
            return q.release();
        }

        // query: construct query
        ISQLQuery* query(const String& expression) throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::query");
            AutoPtr<MySqlQuery> q(new MySqlQuery(m_mysql));
            q->init(expression, true);
            return q.release();
        }

        // update: construct update
        ISQLUpdate* update(const String& expression) throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "MySqlConnection::update");
            AutoPtr<MySqlUpdate> u(new MySqlUpdate(m_mysql));
            u->init(expression);
            return u.release();
        }
    };

    // MySql connection manager
    struct MySql : ISQL
    {
        // Attributes
        String m_host;
        int    m_port;
        String m_db;
        String m_user;
        String m_password;
        ~MySql() { mysql::mysql_library_end(); }

        // init: get default params
        bool init(const Properties& config) 
        {
            Log::Scope scope(KCC_FILE, "init");
            m_host     = config.get("MySql.host",     Strings::empty());
            m_port     = config.get("MySql.port",     k_defPort);
            m_db       = config.get("MySql.db",       Strings::empty());
            m_user     = config.get("MySql.user",     Strings::empty());
            m_password = config.get("MySql.password", Strings::empty());
            Log::info2(
                "MySql config: server=[%s:%d] db=[%s] user=[%s] password=[%s]", 
                m_host.c_str(), m_port, m_db.c_str(), m_user.c_str(), m_password.c_str());
            return true; 
        }

        // connect: connect using init params
        ISQLConnection* connect() throw (SQLException) 
        {
            Log::Scope scope(KCC_FILE, "connect");
            AutoPtr<MySqlConnection> con(new MySqlConnection);
            con->connect(m_host, m_port, m_db, m_user, m_password);
            return con.release();
        }
    };
    
    //
    // MySql factory
    //

    KCC_COMPONENT_FACTORY_IMPL(MySql)
    
    KCC_COMPONENT_FACTORY_METADATA_BEGIN_COMPONENT(MySql, ISQL)
        KCC_COMPONENT_FACTORY_METADATA_PROPERTY(KCC_COMPONENT_FACTORY_SCM, KCC_VERSION)
    KCC_COMPONENT_FACTORY_METADATA_END
}

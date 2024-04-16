/*
 * Kuumba C++ Core
 *
 * $Id: ISQL.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef ISQL_h
#define ISQL_h

namespace kcc
{
    interface ISQLConnection;
    interface ISQLResultSet;
    interface ISQLQuery;
    interface ISQLUpdate;

    /** SQL Exception */
    KCC_COMPONENT_EXCEPTION(SQLException);

    /** SQL date */
    typedef ISODate SQLDate;

    /** SQL long */
    typedef unsigned long long SQLUInt64;

    /**
     * SQL field
     *
     * @author Ted V. Kremer
     */
    interface ISQLField : IComponent
    {
        /** Field type */
        enum Type
        {
            T_SHORT,
            T_LONG,
            T_LONGLONG,
            T_FLOAT,
            T_DOUBLE,
            T_BYTE,
            T_STRING,
            T_DATE,
            T_TIME,
            T_DATETIME,
            T_BINARY    // TODO: NOT IMPLEMENTED
        };

        /** Utility */
        virtual Type   type     () = 0;
        virtual bool   truncated() = 0; // sign or text truncation
        virtual bool   null     () = 0;
        virtual void   null     (bool n) = 0;
        virtual String toString () = 0;

        /** Type-safe Operators */
        virtual short     fShort   ()                   throw (SQLException) = 0;
        virtual long      fLong    ()                   throw (SQLException) = 0;
        virtual long long fLongLong()                   throw (SQLException) = 0;
        virtual float     fFloat   ()                   throw (SQLException) = 0;
        virtual double    fDouble  ()                   throw (SQLException) = 0;
        virtual char      fByte    ()                   throw (SQLException) = 0;
        virtual SQLDate   fDate    ()                   throw (SQLException) = 0;
        virtual SQLDate   fTime    ()                   throw (SQLException) = 0;
        virtual SQLDate   fDateTime()                   throw (SQLException) = 0;
        virtual String    fString  ()                   throw (SQLException) = 0;
        virtual void      fShort   (short sht)          throw (SQLException) = 0;
        virtual void      fLong    (long lng)           throw (SQLException) = 0;
        virtual void      fLongLong(long long ll)       throw (SQLException) = 0;
        virtual void      fFloat   (float flt)          throw (SQLException) = 0;
        virtual void      fDouble  (double dbl)         throw (SQLException) = 0;
        virtual void      fByte    (char byt)           throw (SQLException) = 0;
        virtual void      fDate    (const SQLDate& dat) throw (SQLException) = 0;
        virtual void      fTime    (const SQLDate& tim) throw (SQLException) = 0;
        virtual void      fDateTime(const SQLDate& dt)  throw (SQLException) = 0;
        virtual void      fString  (const String& str)  throw (SQLException) = 0;
    };

    /**
     * SQL facilities
     *
     * @author Ted V. Kremer
     */
    interface ISQL : IComponent
    {
        /**
         * Initialize SQL 
         * @param config sql configuration
         * @return true if initialized
         * @throws SQLException if sql error
         */
        virtual bool init(const Properties& config) = 0;

        /**
         * Connection SQL database
         * @return connection (ownership IS consumed)
         * @throws SQLException if sql error
         */
        virtual ISQLConnection* connect() throw (SQLException) = 0;
    };

    /**
     * SQL Connection
     *
     * @author Ted V. Kremer
     */
    interface ISQLConnection : IComponent
    {
        /**
         * Execute sql
         * @param query expression
         * @return number of rows changed
         * @throws SQLException if sql error
         */
        virtual SQLUInt64 execute     (const String& expression) throw (SQLException) = 0;
        virtual SQLUInt64 executeBatch(const String& expression) throw (SQLException) = 0;
        virtual SQLUInt64 insertId    () = 0;

        /**
         * Transaction sql
         * @throws SQLException if sql error
         */
        virtual void begin   () throw (SQLException) = 0;
        virtual void commit  () throw (SQLException) = 0;
        virtual bool rollback() throw () = 0; // no exception, ok to call from catch()

        /**
         * Construct sql field
         * @param t type of field
         * @return field (ownership IS consumed)
         * @throws SQLException if sql error
         */
        virtual ISQLField* field(ISQLField::Type t) throw (SQLException) = 0;

        /**
         * Query sql
         * @param query expression
         * @return result set/query (ownership IS consumed)
         * @throws SQLException if sql error
         */
        virtual ISQLResultSet* results(const String& expression) throw (SQLException) = 0;
        virtual ISQLQuery*     query  (const String& expression) throw (SQLException) = 0;

        /**
         * Update sql
         * @param update expression
         * @return update (ownership IS consumed)
         * @throws SQLException if sql error
         */
        virtual ISQLUpdate* update(const String& expression) throw (SQLException) = 0;
    };

    /**
     * SQL field collection (managed)
     */
    struct SQLFields
    {
        /** ctor/dtor */
        SQLFields(ISQLConnection* c) : m_con(c) {}
        ~SQLFields() { while (!m_fields.empty()) { delete m_fields.back(); m_fields.pop_back(); } }

        /** Field collection operators */
        inline void add(ISQLField::Type t)    throw (SQLException) { m_fields.push_back(m_con->field(t)); }
        inline ISQLField* operator [] (int n) throw (SQLException) { return at(n); }
        inline int size()                                          { return (int) m_fields.size(); }
        inline ISQLField* at(int n) throw (SQLException)
        {
            if (n >= (int)m_fields.size()) throw SQLException("index out of bounds");
            return m_fields[n]; 
        }

    private:
        SQLFields(const SQLFields&);
        SQLFields& operator = (const SQLFields&);

        // Attributes
        ISQLConnection*         m_con;
        std::vector<ISQLField*> m_fields;
    };

    /**
     * SQL query result set
     *
     * @author Ted V. Kremer
     */
    interface ISQLResultSet : IComponent
    {
        /**
         * Prepare & execute query
         * @param row collection of row field bindings
         * @param params collection of param fields
         * @throws SQLException if sql error
         */
        virtual void prepare(SQLFields& row)                    throw (SQLException) = 0;
        virtual void prepare(SQLFields& row, SQLFields& params) throw (SQLException) = 0;
        virtual void execute()                                  throw (SQLException) = 0;
        virtual void begin  (SQLFields& row)                    throw (SQLException) = 0; // prepare & execute

        /**
         * Query more results
         * @return true if results available
         * @throws SQLException if sql error
         */
        virtual bool next() throw (SQLException) = 0;
    };

    /**
     * SQL result set backed by cursor
     *
     * @author Ted V. Kremer
     */
    interface ISQLQuery : ISQLResultSet
    {
        /** 
         * Position query result set cursor
         * @param row cursor row offset
         * @return number of rows in cursor
         * @throws SQLException if sql error
         */
        virtual SQLUInt64 rows() = 0;
        virtual void      seek(SQLUInt64 row) throw (SQLException) = 0;
    };

    /**
     * SQL update
     *
     * @author Ted V. Kremer
     */
    interface ISQLUpdate : IComponent
    {
        /**
         * Prepare update & execute update
         * @param params collection of param fields
         * @return number of rows changed
         * @throws SQLException if sql error
         */
        virtual void      prepare(SQLFields& params) throw (SQLException) = 0;
        virtual SQLUInt64 execute()                  throw (SQLException) = 0;

        /**
         * Get last insert id
         * @return last insert id
         */
        virtual SQLUInt64 insertId() = 0;
    };
}

#endif // ISQL_h

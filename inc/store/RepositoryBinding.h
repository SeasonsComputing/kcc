/*
 * Kuumba C++ Core
 *
 * $Id: RepositoryBinding.h 20989 2007-10-09 19:32:46Z tvk $
 */
#ifndef RepositoryBinding_h
#define RepositoryBinding_h

#include <inc/store/IXMLRepository.h>

namespace kcc
{
    /**
     * Abstract implementation helper to bind IXMLRepository XML document 
	 * to codegen'd IXMLSerializable value objects.
     *
     *  o Implementation is synchronized
     *  o Value object must have a 'name' attribute that's unique
     *
     * @author Ted V. Kremer
     */
    template<class ResultsT, class AtomicT> class RepositoryBinding : IXMLRepositoryFinder
    {
    public:
        /** Ctor/Dtor */
        RepositoryBinding() {}
        virtual ~RepositoryBinding() {}

        /** 
         * Initialize binding
         * @param repository xmldb repository (ownership IS consumed)
         */
        virtual void init(IXMLRepository* repository)
        {
            m_type = construct()->metadata().type;
            m_repository.reset(repository);
        }
        
        /** 
         * Insert into repository; name must be unique 
         * @param value value to insert
         * @return false if not unique
         * @throws XMLRepositoryException if error
         */
        virtual bool insert(AtomicT& value) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            if (exists(value->name)) return false;
            m_repository->replace(value, value->name);
            return true;
        }
        
        /** 
         * Update value object in repository
         * @param value value to update
         * @return false if not found
         * @throws XMLRepositoryException if error
         */
        virtual bool update(AtomicT& value) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            if (!exists(value->name)) return false;
            m_repository->replace(value, value->name);
            return true;
        }
        
        /** 
         * Replace value object in repository
         * @param value value to replace
         * @throws XMLRepositoryException if error
         */
        virtual void replace(AtomicT& value) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            m_repository->replace(value, value->name);
        }

        /** 
         * Remove value object from repository
         * @param name of object to remove
         * @return false if not found
         * @throws XMLRepositoryException if error
         */
        virtual bool remove(const String& name) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            if (!exists(name)) return false;
            m_repository->remove(name);
            return true;
        }
        
        /** 
         * Find a value object in repository
         * @param value out-param of value object found (first found)
         * @param query find expression
         * @return false if not found
         * @throws XMLRepositoryException if error
         */
        virtual bool find(AtomicT& value, const String& query) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            m_repository->find(this, query);
            if (m_results.empty()) return false;
            value = m_results.front();
            m_results.clear();
            return true;
        }
        
        /** 
         * Find a value object result set in repository
         * @param results out-param of value objects found
         * @param query find expression
         * @throws XMLRepositoryException if error
         */
        virtual void find(ResultsT& results, const String& query, bool clear = true) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            m_repository->find(this, query);
            if (clear) results.clear();
            results.insert(results.end(), m_results.begin(), m_results.end());
            m_results.clear();
        }

        /** 
         * Count of all value objects in repository
         * @param query find expression
         * @return count
         * @throws XMLRepositoryException if error
         */
        virtual long count(const String& query) throw (XMLRepositoryException)
        {
            Mutex::Lock lock(m_sentinel);
            long c = m_repository->count(query);
            return c;
        }
        
        /** 
         * Query if value exists
         * @param query query definition of what to count
         * @param name of object to test existence
         * @return true if exists
         * @throws XMLRepositoryException if error
         */
        inline bool exists(const String& name) throw (XMLRepositoryException)
        {
            return count(byName(name)) != 0L;
        }

        /**
         * Query strings
         */
        inline String byName(const String& n)                  { return byAttr("name", n); }
        inline String byAttr(const String& k, const String& v) { return byAll() + "[@" + k + "='" + v + "']"; }
        inline String byAll()                                  { return "//" + m_type; }

    protected:
        // Attributes
        ResultsT m_results;
        String   m_type;
        Mutex    m_sentinel;
        AutoPtr<IXMLRepository> m_repository;

        // Derived requirement
        virtual AtomicT construct() = 0;
            
        // Implementation
        virtual void onFind(const IDOMNode* node, DOMReader& rdr)
        {
            AtomicT value = construct();
            value->fromXML(node, rdr);
            value->validate();
            m_results.push_back(value);
        }
    };
}

#endif // RepositoryBinding_h

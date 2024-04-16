/*
 * Kuumba C++ Core
 *
 * $Id: AutoPtr.h 20397 2007-08-29 17:41:29Z tvk $
 */
#ifndef AutoPtr_h
#define AutoPtr_h

namespace kcc
{
    /**
     * std::auto_ptr-like class with implicit conversion to ptr type
     */
    template<class _C> class AutoPtr
    {
    public:
        /**
         * Create auto-destructed pointer
         *
         * @param p pointer to auto-destruct
         * @param rhs pointer to attach
         */
        explicit AutoPtr(_C* p = NULL) : m_p(p) {}
        AutoPtr<_C>& operator = (const AutoPtr<_C>& rhs) { reset(rhs.release()); return *this; }
        AutoPtr<_C>& operator = (_C* rhs)                { reset(rhs); return *this; }
        ~AutoPtr() { delete m_p; }

        /**
         * Accessors
         */
        inline _C&       operator *  ()       { return *m_p; }
        inline _C*       operator -> ()       { return m_p;  }
        inline const _C& operator *  () const { return *m_p; }
        inline const _C* operator -> () const { return m_p;  }
        inline operator _C*          ()       { return m_p;  }
        inline operator const _C*    () const { return m_p;  }
        inline bool      null        () const { return m_p == NULL; }

        /**
         * Utility
         */
        inline _C*  release()             { _C* t = m_p; m_p = NULL; return t; }
        inline void reset  (_C* p = NULL) { if (p != m_p && m_p != NULL) { delete m_p; } m_p = p;  }

    private:
        // Attributes
        _C* m_p;
    };

    /**
     * std::auto_ptr-like class with ref-counting and implicit conversion to ptr type
     */
    template<class _C> class SharedPtr
    {
    public:
        /**
         * Create ref-counted auto-descruct pointer
         *
         * @param p pointer to ref-count & auto-destruct 
         * @param rhs pointer to attach
         */
        explicit SharedPtr(_C *p = NULL)    : m_self(new Self(p)) {}
        SharedPtr(const SharedPtr<_C>& rhs) : m_self(NULL)   { attach(rhs.m_self); }
        SharedPtr<_C>& operator = (const SharedPtr<_C>& rhs) { if (this != &rhs) { attach(rhs.m_self); } return *this; }
        SharedPtr<_C>& operator = (_C* rhs) { reset(rhs); return *this; }
        ~SharedPtr() { clean(); }

        /**
         * Accessors
         */
        inline _C& operator *        ()       { return *m_self->m_p; }
        inline _C* operator ->       ()       { return m_self->m_p;  }
        inline const _C& operator *  () const { return *m_self->m_p; }
        inline const _C* operator -> () const { return m_self->m_p;  }
        inline operator _C*          ()       { return m_self->m_p;  }
        inline operator const _C*    () const { return m_self->m_p;  }
        inline bool      null        () const { return m_self == NULL || m_self->m_p == NULL; }

        /**
         * Utility
         */
        inline _C*  release()             { _C* t = m_self->m_p; clean(false); return t; }
        inline void reset  (_C* p = NULL) { clean(); m_self = new Self(p); }
        inline int  rc     () const       { return m_self->m_rc; }

    private:
        // Attributes
        struct Self
        {
            Self(_C* p) : m_rc(1), m_p(p) {}
            ~Self() { delete m_p; }
            int m_rc;
            _C* m_p;
        } *m_self;

        // Implemenation
        inline void attach(Self* t)
        {
            if (m_self != NULL) clean();
            m_self = t;
            m_self->m_rc++;
        }
        inline void clean(bool del = true) 
        { 
            m_self->m_rc--; 
            if (m_self->m_rc==0 && del) 
            {
                delete m_self; 
                m_self = NULL;
            }
        }
    };
}

#endif // AutoPtr_h

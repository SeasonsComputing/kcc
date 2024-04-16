/*
 * Kuumba C++ Core
 *
 * $Id: Exception.h 15199 2007-03-09 17:57:17Z tvk $
 */
#ifndef Exception_h
#define Exception_h

namespace kcc
{
    /**
     * Exception class that defines C-string behavior
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT Exception : public std::exception
    {
    public:
        /** Exception construction */
        Exception(const String& msg) throw ();
        virtual ~Exception() throw ();

        /** Accessor to what field */
        const char* what() const throw ();

    private:
        // Attributes
        String m_msg;
    };
}

#endif // Exception_h

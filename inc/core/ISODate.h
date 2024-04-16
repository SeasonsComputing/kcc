/*
 * Kuumba C++ Core
 *
 * $Id: ISODate.h 22625 2008-03-09 22:51:49Z tvk $
 */
#ifndef ISODate_h
#define ISODate_h

namespace kcc
{
    /**
     * ISO Date ADT
     *  year   - whole year
     *  month  - 1 to 12
     *  day    - 1 to 31
     *  hour   - 0 to 24
     *  minute - 0 to 59
     *  second - 0 to 59
     *
     * @author Ted V. Kremer
     */
    class KCC_CORE_EXPORT ISODate
    {
    public:
        /** Attributes */
        ISODate() : year(0), month(0), day(0), hour(0), minute(0), second(0) {}
        ISODate(int y, int mo, int d, int h=0, int m=0, int s=0) : year(y), month(mo), day(d), hour(h), minute(m), second(s) {}
        ISODate(const String& iso) { *this = ISODate::iso(iso); }
        int year, month, day, hour, minute, second;

        /** Utility */
        inline operator std::time_t () const { return ISODate::time(*this); }
        inline int compare(const ISODate& rhs) const { return rawdatetime().compare(rhs.rawdatetime()); }
        inline bool valid_t() const { return valid() && (year >= 1970 && year <= 2038); }
        inline bool valid() const
        {
            return
                (year   >  0)                 &&
                (month  >  0 && month  <= 12) &&
                (day    >  0 && day    <= 31) &&
                (hour   >= 0 && hour   <= 24) &&
                (minute >= 0 && minute <  60) &&
                (second >= 0 && second <  60);
        }
        inline bool null() const
        {
            return
                year == 0 && month == 0 && day == 0 &&
                hour == 0 && minute == 0 && second == 0;
        }

        /** 
         * ISO formatting: 
         *   Time zone and zulu formatting assume a local time and will be
         *   converted to UTC automatically when writing TZ or Z formats
         */
        enum Format { F_NONE, F_TIMEZONE, F_ZULU };
        
        /** ISO extended format (YYYY-MM-DDTHH:MM:SSZ+/-HH:MM) */
        String isodate    () const;
        String isotime    () const;
        String isodatetime(Format f = ISODate::F_NONE) const;

        /** ISO basic format (YYYYMMDDTHHMMSSZ+/-HHMM) */
        String rawdate    () const;
        String rawtime    () const;
        String rawdatetime(Format f = ISODate::F_NONE) const;

        /** GMT format (Sun, 08 Feb 1971 19:57:13 GMT) */
        String gmtdatetime() const;

        /** Factory */
        static std::time_t time (const ISODate& d);       // to time_t (assumes d is local time)
        static ISODate     tm   (const std::tm& tm);      // from struct tm
        static ISODate     iso  (const String& iso);      // from extended ISO
        static ISODate     raw  (const String& raw);      // from basic ISO
        static ISODate     local(const std::time_t& t);   // from time_t (assumes t is local time; no conversion)
        static ISODate     utc  (const std::time_t& t);   // from time_t (assumes t is local time; will convert to utc)
        static ISODate     local();                       // from current local date time
        static ISODate     utc  ();                       // from current utc date time
        
        /** Utility */
        static bool valid(const String& iso); // validates format NOT value
    };
    inline bool operator <  (const ISODate& lhs, const ISODate& rhs) { return lhs.compare(rhs) <  0; }
    inline bool operator == (const ISODate& lhs, const ISODate& rhs) { return lhs.compare(rhs) == 0; }
    inline std::ostream& operator << (std::ostream& out, const ISODate& d) { out << d.isodatetime(); return out; }
};

#endif // ISODate_h

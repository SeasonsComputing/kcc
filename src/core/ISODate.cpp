/*
 * Kuumba C++ Core
 *
 * $Id: ISODate.cpp 23041 2008-04-07 20:09:50Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   define kcc_gmtime(_clock,_result) (*(_result)=*gmtime((_clock)))
#   define kcc_localtime(_clock,_result) (*(_result)=*localtime((_clock)))
#   define kcc_tzset() _tzset()
#   define kcc_timezone() _timezone
#elif defined (KCC_LINUX)
#   define kcc_gmtime gmtime_r
#   define kcc_localtime localtime_r
#   define kcc_tzset() tzset()
#   define kcc_timezone() timezone
#endif

#define KCC_FILE "ISODate"

namespace kcc
{
    // Constants
    static const String k_rx_date    ("^[0-9]{4,4}-[0-9]{2,2}-[0-9]{2,2}$");
    static const String k_rx_time    ("^[0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}([\\+-]{1,1}[0-9]{2,2}:[0-9]{2,2}|[Z]){0,1}$");
    static const String k_rx_datetime("^[0-9]{4,4}-[0-9]{2,2}-[0-9]{2,2}[ T]{1,1}[0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}([\\+-]{1,1}[0-9]{2,2}:[0-9]{2,2}|[Z]){0,1}$");

    // Char stack buffer size
    const std::size_t SZ = 64;

    // isodate: get ISO formatted date
    String ISODate::isodate() const
    {
        Char buf[SZ];
        std::sprintf(buf, "%04d-%02d-%02d", year, month, day);
        return buf;
    }

    // isotime: get ISO formatted time
    String ISODate::isotime() const
    {
        Char buf[SZ];
        std::sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
        return buf;
    }

    // isodatetime: get ISO formatted date & time
    String ISODate::isodatetime(Format f) const
    {
        Char buf[SZ];
        if (f == ISODate::F_TIMEZONE)
        {
            // time zone: assume local time, convert to utc w/ tz
            std::time_t local = *this;
            ISODate utc(ISODate::utc(local));
            long tzs = (long)(local - (std::time_t)utc);
            long tzm = tzs/60L;
            long tzh = tzm/60L;
            tzm -= (tzh*60L);
            std::sprintf(
                buf, 
                "%04d-%02d-%02dT%02d:%02d:%02d%+03d:%02d", 
                utc.year, utc.month, utc.day, 
                utc.hour, utc.minute, utc.second, 
                (int)tzh, (int)tzm);
        }
        else if (f == ISODate::F_ZULU)
        {
            // zulu zone: assume local time, convert to utc
            ISODate utc(ISODate::utc(*this));
            std::sprintf(
                buf, 
                "%04d-%02d-%02dT%02d:%02d:%02dZ", 
                utc.year, utc.month, utc.day, 
                utc.hour, utc.minute, utc.second);
        }
        else
        {
            std::sprintf(
                buf, 
                "%04d-%02d-%02d %02d:%02d:%02d", 
                year, month, day, hour, minute, second);
        }
        return buf;
    }

    // rawdate: get raw formatted date
    String ISODate::rawdate() const
    {
        Char buf[SZ];
        std::sprintf(buf, "%04d%02d%02d", year, month, day);
        return buf;
    }

    // rawtime: get raw formatted time
    String ISODate::rawtime() const
    {
        Char buf[SZ];
        std::sprintf(buf, "%02d%02d%02d", hour, minute, second);
        return buf;
    }

    // rawdatetime: get raw formatted date & time
    String ISODate::rawdatetime(Format f) const
    {
        Char buf[SZ];
        if (f == ISODate::F_TIMEZONE)
        {
            // time zone: assume local time, convert to utc w/ timezone
            std::time_t local = *this;
            ISODate utc(ISODate::utc(local));
            long tzs = (long)(local - (std::time_t)utc);
            long tzm = tzs/60L;
            long tzh = tzm/60L;
            tzm -= (tzh*60L);
            std::sprintf(
                buf, 
                "%04d%02d%02d%02d%02d%02d%+03d%02d", 
                utc.year, utc.month, utc.day, 
                utc.hour, utc.minute, utc.second, 
                (int)tzh, (int)tzm);
        }
        else if (f == ISODate::F_ZULU)
        {
            // zulu: assume local time, convert to utc
            ISODate utc(ISODate::utc(*this));
            std::sprintf(
                buf, 
                "%04d%02d%02d%02d%02d%02d", 
                utc.year, utc.month, utc.day, 
                utc.hour, utc.minute, utc.second);
        }
        else
        {
            std::sprintf(buf, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);
        }
        return buf;
    }
    
    // gmtdatetime: gmt formatted date time
    String ISODate::gmtdatetime() const
    {
        // Sun, 02 Apr 2006 19:57:13 GMT
        std::time_t time = *this;
        Char buf[SZ];
        std::strftime(buf, SZ, "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&time));
        return buf;
    }
    
    // time: convert to time_t
    std::time_t ISODate::time(const ISODate& d)
    {
        if (d.null()) return (std::time_t)0;
        std::tm tm  = {0};
        tm.tm_year  = d.year - 1900;
        tm.tm_mon   = d.month - 1;
        tm.tm_mday  = d.day;
        tm.tm_hour  = d.hour;
        tm.tm_min   = d.minute;
        tm.tm_sec   = d.second;
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    }

    // tm: constuct from std tm struct
    ISODate ISODate::tm(const std::tm& tm)
    {
        return ISODate(
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);
    }

    // iso: construct from ISO format
    ISODate ISODate::iso(const String& iso)
    {
        ISODate d;
        String::size_type sz = iso.length();
        if (sz == 10)
        {
            std::sscanf(iso.c_str(), "%04d-%02d-%02d", &d.year, &d.month, &d.day);
        }
        else if (sz == 8)
        {
            std::sscanf(iso.c_str(), "%02d:%02d:%02d", &d.hour, &d.minute, &d.second);
        }
        else if (sz == 19 || sz == 20) // 20 for optional trailing Z for zulu/utc
        {
            // extended ISO format
            char t = 0;
            std::sscanf(iso.c_str(), "%04d-%02d-%02d%c%02d:%02d:%02d", &d.year, &d.month, &d.day, &t, &d.hour, &d.minute, &d.second);
            
            // conver to local
            if (sz == 20) 
            {
                std::time_t loc = ISODate::time(d);
                std::time_t utc = ISODate::utc(loc);
                std::time_t tz  = (std::time_t)(utc-loc);
                d = ISODate::local(loc - tz);
            }
        }
        else
        {
            // extended ISO format w/ time zone
            char  t = 0;
            float s = 0.0F;
            int   zh = 0, zm = 0;
            std::sscanf(iso.c_str(), "%04d-%02d-%02d%c%02d:%02d:%f%03d:%02d", &d.year, &d.month, &d.day, &t, &d.hour, &d.minute, &s, &zh, &zm);

            // truncate fractional seconds (TODO: fraction time in ISOTime)
            d.second = (int)s; 
            
            // convert to local
            if (zh != 0 || zm != 0) 
            {
                std::time_t utc = ISODate::time(d);
                std::time_t tz  = (std::time_t)(((zh*60L)+zm)*60L);
                d = ISODate::local(utc + tz);
            }
        }
        return d;
    }

    // raw: construct from raw format
    ISODate ISODate::raw(const String& raw)
    {
        ISODate d;
        String::size_type sz = raw.length();
        if (sz == 8)
            std::sscanf(raw.c_str(), "%04d%02d%02d", &d.year, &d.month, &d.day);
        else if (sz == 6)
            std::sscanf(raw.c_str(), "%02d%02d%02d", &d.hour, &d.minute, &d.second);
        else
            std::sscanf(raw.c_str(), "%04d%02d%02d%02d%02d%02d", &d.year, &d.month, &d.day, &d.hour, &d.minute, &d.second);
        return d;
    }

    // local: build current local iso datetime
    ISODate ISODate::local(const std::time_t& t) 
    { 
        if (t < 0) return ISODate();
        std::tm tm = {0};
        kcc_localtime(&t, &tm);
        return ISODate::tm(tm); 
    }

    // utc: build current local time_t to zulu iso datetime
    ISODate ISODate::utc(const std::time_t& t) 
    {
        if (t < 0) return ISODate();
        std::tm tm = {0};
        kcc_gmtime(&t, &tm);
        return ISODate::tm(tm); 
    }

    // local: construct from current local date time
    ISODate ISODate::local()
    {
        return ISODate::local(std::time(NULL));
    }

    // utc: construct from zulu date time
    ISODate ISODate::utc()
    {
        return ISODate::utc(std::time(NULL));
    }
    
    // valid: validate string format of ISO date
    bool ISODate::valid(const String& iso)
    {
        try
        {
            IRegex* rx = Core::regex();
            return
                rx->match(iso, k_rx_datetime) ||
                rx->match(iso, k_rx_date)     ||
                rx->match(iso, k_rx_time);
        }
        catch (Exception&)
        {
            return false;
        }
    }
}

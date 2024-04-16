/*
 * Kuumba C++ Core
 *
 * $Id: Timer.h 20311 2007-08-20 20:11:13Z tvk $
 */
#ifndef Timer_h
#define Timer_h

namespace kcc
{
    /**
     * Timer ADT
     *
     * USAGE:
     *
     *   kcc::Timer t;
     *   t.start();
     *   ... work ...
     *   t.stop();
     *   double totSecs = t.secs();
     *   ... OR ...
     *   double totSecs = t.now(); // stop() then secs()   
     *
     * @author Ted V. Kremer
     */
    struct Timer
    {
        // Attributes
        double initial;
        double elapsed;
        inline Timer() : initial(0.0), elapsed(0.0) {}
        
        // Services
        inline void   start()      { initial = Platform::procTimeInUseSecs(); }
        inline void   stop()       { elapsed += (Platform::procTimeInUseSecs()-initial); }
        inline double secs() const { return elapsed; }
        inline double now()        { stop(); return secs(); }
        inline void   reset()      { initial = elapsed = 0L; }
    };
    struct TimerLess
    {
	    inline bool operator () (const Timer& l, const Timer& r) const { return l.secs() < r.secs(); } 
    };    
    
    /**
     * Timers Stack ADT
     *
     * USAGE:
     *
     *   kcc::Timers t;
     *   t.start(); // push then top.start()
     *   ... work ...
     *   t.stop();  // top.stop()
     *   double totSecs = t.secs();
     *   ... OR ...
     *   double totSecs = t.now(); // top.stop() then top.secs()   
     *
     *   double avgSecs = t.mean();
     *   double sumSecs = t.sum();
     *
     * @author Ted V. Kremer
     */
    struct Timers
    {
        // Attributes
        typedef std::vector<Timer> Collection;
        const Char* name;
        Collection  timers;
        inline Timers(const Char* n = "") : name(n) { timers.reserve(16); }
        
        // Services
        inline void   start()      { push().start(); }
        inline void   stop()       { top().stop(); }
        inline double secs() const { return top().secs(); }
        inline double now()        { return top().now(); }
        inline void   reset()      { top().reset(); }

        // Reporting
        inline Collection::size_type size() const { return timers.size(); }
        inline double sum() const
        { 
            double s = 0.0F; 
            for (Collection::const_iterator i = timers.begin(); i != timers.end(); i++) s += i->secs();
            return s;
        }
        inline double mean() const
        {
            if (empty()) return 0.0;
            else         return sum() / size(); 
        }
        inline double median() const 
        {
            if (empty()) return 0.0;
            Collection::size_type sz = size();
            if (sz < 2) return timers[0].secs();
            Collection sorted(timers);
            std::sort(sorted.begin(), sorted.end(), TimerLess());
            Collection::size_type mid = sz / 2;
            if (sz % 2 == 0) return (sorted[mid-1].secs() + sorted[mid].secs()) / 2.0;
            else             return sorted[mid].secs();
        }
        
        // Utility
        inline Timer&       push()        { timers.push_back(Timer()); return timers.back(); }
        inline void         pop()         { timers.pop_back(); }
        inline Timer&       top()         { return timers.back(); }
        inline const Timer& top()   const { return timers.back(); }
        inline bool         empty() const { return timers.empty(); }
    };
}

#endif // Timer_h

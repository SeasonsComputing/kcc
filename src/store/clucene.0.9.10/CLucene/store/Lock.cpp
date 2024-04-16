#include "CLucene/StdHeader.h"
#include "Lock.h"

CL_NS_DEF(store)

   bool LuceneLock::obtain(int64_t lockWaitTimeout) {
      bool locked = obtain();
      int maxSleepCount = (int)(lockWaitTimeout / LOCK_POLL_INTERVAL);
      int sleepCount = 0;
      while (!locked) {
         if (++sleepCount == maxSleepCount) {
            _CLTHROWA(CL_ERR_IO,"Lock obtain timed out");
         }
         _sleep(LOCK_POLL_INTERVAL);
         locked = obtain();
      }
      return locked;
   }

    void* LuceneLockWith::run() {
        bool locked = false;
      void* ret = NULL;
#ifndef CLUCENE_LITE
        try {
          locked = lock->obtain(lockWaitTimeout);
#endif
            ret = doBody();
#ifndef CLUCENE_LITE
          }_CLFINALLY(
              if (locked) lock->release();
          );
#endif
      return ret;
      }
CL_NS_END

/*
 * Kuumba C++ Core
 *
 * $Id: IBrowserHandler.h,v 1.1 2003/11/25 05:31:35 tedk Exp $
 */
#ifndef IBrowserHandler_h
#define IBrowserHandler_h

namespace kcc
{
    /**
     * Interface handler for browser events
     *
     * @author Ted V. Kremer
     */
    class IBrowserHandler
    {
    public:
        /**
        * Notification of browser link clicked
        * @param link link clicked on
        * @param frame target name of link
        * @param post data of link
        * @return true if link is consumed (don't let browser follow lnk)
        */
        virtual bool onBrowserLink(const String& link, const String& frame, const String& post) = 0;
    };
}

#endif // IBrowserHandler_h

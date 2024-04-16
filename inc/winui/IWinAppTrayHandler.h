/*
 * Kuumba C++ Core
 *
 * $Id: IWinAppTrayHandler.h,v 1.5 2004/01/07 03:34:33 tedk Exp $
 */
#ifndef IWinAppTrayHandler_h
#define IWinAppTrayHandler_h

namespace kcc
{
    /**
     * Interface handler for task tray app
     *
     *    Handler Providers Must Dispatch In Order
     *      1. onTraySetup()  - 1 and only 1
     *      2. onTrayInit()   - 1 and only 1
     *      3. onTrayCommand(), onTrayLink() - as fired
     *      4. onTrayExit()   - until returns true
     *
     * @author Ted V. Kremer
     */
    class IWinAppTrayHandler
    {
    public:
        /** Resource IDS for handler */
        enum Commands
        { 
            C_RANGE_START = 100,
            C_RANGE_END   = 300
        };

        /** Collection of icon identifiers */
        typedef std::vector<USHORT> Icons;

        /** Structure to manage state for task tray handler */
        struct Config
        {
            // Attributes
            USHORT idResource;
            USHORT idIconTaskTray;
            Icons  idIconsTaskTrayAnimate;
            USHORT idMenuDefault;
            USHORT idMenuExit;
            String urlDefault;
            Config() : 
                idResource(0), idIconTaskTray(0), idIconsTaskTrayAnimate(), 
                idMenuDefault(0), idMenuExit(), urlDefault()
            {}
        };

        /**
         * Configure application
         * @param props properties to initialize
         */
        virtual void onTraySetup(gpsProperties& props, IWinAppTrayHandler::Config& config) = 0;

        /**
         * Notification of tray initialization
         * @return true if ok to continue app
         */
        virtual bool onTrayInit() = 0;

        /**
         * Notification of tray exit
         * @param force force shutdown of the app (the OS is exiting)
         * @return true if ok to exit
         */
        virtual bool onTrayExit(bool force) = 0;

        /**
         * Notification of tray command
         * @param cmd command
         */
        virtual void onTrayCommand(int cmd) = 0;

        /**
         * Notification of tray window link
         * @param link link clicked on
         * @param frame target name of link
         * @param post data of link
         * @return true if link is consumed (don't let browser follow lnk)
         */
        virtual bool onTrayLink(const String& link, const String& frame, const String& post) = 0;

        /**
         * Notification of tray popup
         */
        virtual void onTrayMenuPopup() = 0;
    };
}

#endif // IWinAppTrayHandler_h

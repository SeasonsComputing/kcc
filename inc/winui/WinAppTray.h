/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTray.h,v 1.5 2004/01/07 03:34:33 tedk Exp $
 */
#ifndef WinTrayApp_h
#define WinTrayApp_h

#include <inc/winui/IWinAppTrayHandler.h>

namespace kcc
{
    class WinAppTrayModule;

    /**
     * Windows tray application
     *  - provides a task tray icon and menu
     *  - a tray main window (e.g. configuration)
     *  - tray window is populated with a browser and contents
     *    are defined in MSHTML
     *
     * @author Ted V. Kremer
     */
    class WinAppTray : protected IWinAppTrayHandler
    {
    public:
        /**
         * Accessor to application singleton
         */
        static WinAppTray& instance();

        /**
         * Run tray app
         * @param h instance to use for resources
         */
        virtual int trayRun(HINSTANCE h);

        /**
         * Exit tray app
         */
        virtual void trayExit();

        /**
         * Begin animating tray icons
         */
        virtual void trayAnimateStart();

        /**
         * Finish animating tray icons (return to normal)
         */
        virtual void trayAnimateStop();

        /**
         * Menu enable/disable
         *
         * @param id id of menu item
         * @param enabled enable status of menu item
         */
        virtual void trayMenuEnable(USHORT id, bool enabled);

        /**
         * Open tray window to default or to link
         * @param link link to open
         */
        virtual void trayOpen(const String& link);

        /**
         * Close tray default window
         */
        virtual void trayClose();

        /** 
         * Application Support 
         */
        virtual HWND trayHWND();
        virtual void messageOk (const String& msg);
        virtual void messageErr(const String& msg);
        virtual bool messageAsk(const String& msg);
        virtual void explorer(const String& url);

        /**
         * Timer Support
         */
        virtual void startTimer(int repeat, int id);
        virtual void killTimer (int id);

        /** 
         * Accessors
         */
        const IWinAppTrayHandler::Config& config() { return m_config; }
        const String&                     path()   { return m_path; }

        /**
         * Query argument value 
         * @param arg argument to get value for
         * @param value out param with value for argument
         * @return true if arg was found in argument list
         */
        virtual bool argValue(const String& arg, String& value);

    protected:
        // Implementation
        WinAppTray();
        virtual ~WinAppTray();

        // Tray Handlers
        virtual bool onTrayInit();
        virtual void onTrayCommand(int cmd);
        virtual bool onTrayLink(const String& link, const String& frame, const String& post);
        virtual void onTrayMenuPopup();

        // Helpers
        virtual void parseCommandLine();

    private:
        // Timer Handler
        static void onTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

        // Attributes
        typedef std::map<int, int> Timers;
        static WinAppTray*           k_instance;
        IWinAppTrayHandler::Config   m_config;
        String                       m_path;
        StringVector                 m_args;
        HANDLE                       m_mutex; // only 1 instance of app
        AutoPtr<WinAppTrayModule>    m_module;
        Timers                       m_timersById;
        Timers                       m_timersByTimer;

        WinAppTray(const WinAppTray&);
        WinAppTray& operator = (const WinAppTray&);
    };
}

#endif // WinTrayApp_h

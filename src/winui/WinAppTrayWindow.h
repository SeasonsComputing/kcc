/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTrayWindow.h,v 1.5 2004/01/07 04:42:07 tedk Exp $
 */
#ifndef WinAppTrayWindow_h
#define WinAppTrayWindow_h

#include <src/winui/IBrowserHandler.h>

namespace kcc
{
    class BrowserHandler;

    /**
     * Implementation of tray app window
     *
     * @author Ted V. Kremer
     */
    class WinAppTrayWindow : public ATL::CAxDialogImpl<WinAppTrayWindow>, IBrowserHandler
    {
    public:
        /**
         * Ctor/dtor
         * @param cb handler callback (NULL NOT VALID)
         */
        WinAppTrayWindow(IWinAppTrayHandler* handler);
        virtual ~WinAppTrayWindow();

        /**
         * Initialize tray window
         * @param config configuration
         * @return true if initialized ok
         */
        virtual bool init(const IWinAppTrayHandler::Config& config);

        /**
         * Call to enable event listening in task tray
         */
        virtual void loaded();

        /**
         * Open tray window to link
         * @param link link to open
         */
        virtual void open(const String& link);

        /**
         * Close tray window
         */
        virtual void close();

        /**
         * Animate tray icons
         */
        virtual void animateStart();

        /**
         * Stop animating tray icons
         */
        virtual void animateStop();

        /**
         * Menu enable/disable
         *
         * @param id id of menu item
         * @param enabled enable status of menu item
         */
        virtual void menuEnable(USHORT id, bool enabled);

    protected:
        // Implementation
        virtual void popup();
        virtual bool onBrowserLink(const String& link, const String& frame, const String& post);
        virtual bool getTrayIcon(USHORT id, HICON& icon, String& tip);
        virtual bool setTrayIcon(int ndx, bool create=false);

        // Window callbacks
        enum 
        { 
            MSG_ICON_NOTIFY = WM_USER + 100,
            MSG_TIMER       = WM_USER + 101
        };
        BEGIN_MSG_MAP(WinAppTrayWindow)
            MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, onDestroy)
            MESSAGE_HANDLER(MSG_ICON_NOTIFY, onIconNotify)
            MESSAGE_HANDLER(WM_TIMER, onTimer)
            MESSAGE_HANDLER(WM_ENDSESSION, onEndSession)
            MESSAGE_HANDLER(m_taskbarRestart, onTrayRestart)
            COMMAND_RANGE_HANDLER(IWinAppTrayHandler::C_RANGE_START, IWinAppTrayHandler::C_RANGE_END, onCommand)
            COMMAND_ID_HANDLER(IDCANCEL, onCancel)
        END_MSG_MAP()
        LRESULT onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onTrayRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT onCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

        // Attributes
        USHORT                    IDD;
        IWinAppTrayHandler*       m_handler;
        HICON                     m_icon;
        HMENU                     m_popup;
        IWinAppTrayHandler::Icons m_iconsTaskTray;
        StringVector              m_tipsTaskTray;
        AutoPtr<BrowserHandler>   m_browser;
        UINT                      m_timer;
        USHORT                    m_defMenu;
        int                       m_ndxTrayIcon;
        bool                      m_loading;
        int                       m_aniRefCount;
        Mutex                     m_aniMutex;
        UINT                      m_taskbarRestart;
    };
}

#endif // WinAppTrayWindow_h

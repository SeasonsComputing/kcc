/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTrayWindow.cpp,v 1.7 2004/01/07 04:42:07 tedk Exp $
 */
#include <inc/core/Core.h>
#include <inc/winui/IWinAppTrayHandler.h>
#include <src/winui/winui.h>
#include <src/winui/BrowserHandler.h>
#include <src/winui/WinAppTrayWindow.h>

#define KCC_FILE "WinAppTrayWindow"

namespace kcc
{
    // refresh rate for animated icons
    static const long k_flash = 333L;

    // ctor/dtor
    WinAppTrayWindow::WinAppTrayWindow(IWinAppTrayHandler* handler) : 
        IDD(0), m_handler(handler), m_icon(0), m_popup(0),
        m_browser(new BrowserHandler(this)), m_timer(0), 
        m_ndxTrayIcon(0), m_loading(true), m_aniRefCount(0),
        m_taskbarRestart(0)
    {}
    WinAppTrayWindow::~WinAppTrayWindow() {}

    // init: create app window from config
    bool WinAppTrayWindow::init(const IWinAppTrayHandler::Config& config)
    {
        Log::Scope scope(KCC_FILE, "init");

        // intialize state and create window
        IDD       = config.idResource;
        m_defMenu = config.idMenuDefault;
        if (ATL::Create(NULL) == NULL) 
        {
            Log::error(gpsPlatformUtil::lastError().c_str());
            return false;
        }

        // load app resources
        m_icon = ::LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDD));
        HMENU menu = ::LoadMenu(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDD));
        if (m_icon == NULL || menu == NULL) 
        {
            Log::error("Can't create icon or menu: %d", IDD);
            return false;
        }
        SetIcon(m_icon);

        // get popup menu
        m_popup = ::GetSubMenu(menu, 0);
        if (m_defMenu != NULL) ::SetMenuDefaultItem(m_popup, m_defMenu, FALSE);

        // load task tray resource
        HICON icon;
        String tip;
        if (!getTrayIcon(config.idIconTaskTray, icon, tip)) 
        {
            Log::error("Can't load tray icon: %d", config.idIconTaskTray);
            return false;
        }
        m_iconsTaskTray.push_back((USHORT)icon);
        m_tipsTaskTray.push_back(tip);

        // load busy icons
        for (
            IWinAppTrayHandler::Icons::const_iterator i = config.idIconsTaskTrayAnimate.begin();
            i != config.idIconsTaskTrayAnimate.end();
            i++)
        {
            if (!getTrayIcon(*i, icon, tip)) 
            {
                Log::error("Can't load busy tray icon: %d", *i);
                return false;
            }
            m_iconsTaskTray.push_back((USHORT)icon);
            m_tipsTaskTray.push_back(tip);
        }

        // intialize task tray   
        return setTrayIcon(m_ndxTrayIcon, true);
    }

    // loaded: app is loaded
    void WinAppTrayWindow::loaded() { m_loading = false; }

    // close: close tray window
    void WinAppTrayWindow::close() { ShowWindow(SW_HIDE); }

    // open: open tray window
    void WinAppTrayWindow::open(const String& link)
    {
        m_browser->navigate(link);
        SetWindowPos(HWND_TOP, 0, 0, 600, 500, SWP_NOMOVE);
        if (!IsWindowVisible()) CenterWindow(::GetDesktopWindow());
        ShowWindow(SW_SHOW);
    }

    // animateStart: start animating tray
    void WinAppTrayWindow::animateStart()
    {
        Mutex::Lock lock(m_aniMutex);
        m_aniRefCount++;
        if (m_aniRefCount == 1 && m_iconsTaskTray.size() > 1)
        {
            m_ndxTrayIcon = 1; // [0] is tray app icon
            setTrayIcon(m_ndxTrayIcon);
            m_timer = SetTimer(MSG_TIMER, k_flash);
        }
    }

    // animateStop: stop animating tray
    void WinAppTrayWindow::animateStop()
    {
        Mutex::Lock lock(m_aniMutex);
        m_aniRefCount--;
        if (m_aniRefCount == 0)
        {
            KillTimer(m_timer);
            m_timer = 0;
            m_ndxTrayIcon = 0;
            setTrayIcon(m_ndxTrayIcon);
        }
    }

    // menuEnable: set popup menu item enable status
    void WinAppTrayWindow::menuEnable(USHORT id, bool enabled)
    {
        MENUITEMINFO mi = {0};
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask  = MIIM_STATE;
        mi.fState = (enabled) ? MFS_ENABLED : MFS_DISABLED;
        ::SetMenuItemInfo(m_popup, id, false, &mi);
    }

    // popup: show tray popup
    void WinAppTrayWindow::popup()
    {
        m_handler->onTrayMenuPopup();
        CPoint p;
        ::GetCursorPos(&p);
        ::SetForegroundWindow(m_hWnd);
        ::TrackPopupMenuEx(m_popup, TPM_BOTTOMALIGN, p.x, p.y, m_hWnd, NULL);
    }

    // onBrowserLink: delegate to handler
    bool WinAppTrayWindow::onBrowserLink(const String& link, const String& frame, const String& post)
    { 
        return m_handler->onTrayLink(link, frame, post); 
    }

    // onInitDialog: initialize window
    LRESULT WinAppTrayWindow::onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        m_taskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
        m_browser->attach(this);
        return 1;
    }

    // onExitDialog: clean up window on exit
    LRESULT WinAppTrayWindow::onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (IDD == 0) return 0; // don't clean more than once
        m_browser->detach();
        NOTIFYICONDATA iconData = {0};
        iconData.cbSize = sizeof(NOTIFYICONDATA);
        iconData.hWnd    = m_hWnd;
        iconData.uID    = MSG_ICON_NOTIFY;
        Shell_NotifyIcon(NIM_DELETE, &iconData);
        ::DestroyMenu(m_popup);
        ::DestroyIcon(m_icon);
        for (
            IWinAppTrayHandler::Icons::iterator i = m_iconsTaskTray.begin();
            i != m_iconsTaskTray.end();
            i++)
        {
            ::DestroyIcon((HICON)*i);
        }
        IDD = 0;
        return 0;
    }

    // onIconNotify: delegate to commanf handler
    LRESULT WinAppTrayWindow::onIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (m_loading) return 0;
        switch (lParam)
        {
        case WM_LBUTTONUP: (m_defMenu != 0) ? m_handler->onTrayCommand(m_defMenu) : popup(); break;
        case WM_RBUTTONUP: popup(); break;
        }
        return 0;
    }

    // onTimer: animate trask tray icons
    LRESULT WinAppTrayWindow::onTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        m_ndxTrayIcon++;
        if (m_ndxTrayIcon >= (int)m_iconsTaskTray.size()) m_ndxTrayIcon = 1; // [0] is tray app icon
        setTrayIcon(m_ndxTrayIcon);
        return 0;
    }

    // onEndSession: force shutdown of app
    LRESULT WinAppTrayWindow::onEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (wParam==TRUE) m_handler->onTrayExit(true);
        return 0;
    }

    // onTrayRestart: redisplay icon
    LRESULT WinAppTrayWindow::onTrayRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        setTrayIcon(m_ndxTrayIcon, true);
        return 0;
    }

    // onCommand: delegate to command handler
    LRESULT WinAppTrayWindow::onCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_handler->onTrayCommand(wID);
        return 0;
    }

    // onCancel: close window
    LRESULT WinAppTrayWindow::onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        close();
        return 0;
    }

    // getTrayIcon: get tray icon and tool tip
    bool WinAppTrayWindow::getTrayIcon(USHORT id, HICON& icon, String& tip)
    {
        icon = ::LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(id));
        tip.assign(gpsPlatformUtil::loadText(id));
        return icon != NULL;
    }

    // setTrayIcon: set tray icon and tool tip
    bool WinAppTrayWindow::setTrayIcon(int ndx, bool create)
    {
        KCC_ASSERT(ndx < (int)m_iconsTaskTray.size(), KCC_FILE, "setTrayIcon", "Icon index out of bounds");
        NOTIFYICONDATA iconData   = {0};
        iconData.cbSize              = sizeof(NOTIFYICONDATA);
        iconData.hWnd              = m_hWnd;
        iconData.uID              = MSG_ICON_NOTIFY;
        iconData.uFlags              = NIF_MESSAGE|NIF_ICON|NIF_TIP;
        iconData.uCallbackMessage = MSG_ICON_NOTIFY;
        iconData.hIcon              = (HICON)m_iconsTaskTray[ndx];
        ::lstrcpyn(iconData.szTip, m_tipsTaskTray[ndx].c_str(), (int)m_tipsTaskTray[ndx].size()+1);
        return Shell_NotifyIcon((create) ? NIM_ADD : NIM_MODIFY, &iconData) == TRUE;
    }
}

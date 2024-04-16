/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTray.cpp,v 1.8 2004/01/07 03:34:07 tedk Exp $
 */
#include <inc/core/Core.h>
#include <inc/winui/IWinAppTrayHandler.h>
#include <inc/winui/WinAppTray.h>
#include <src/winui/winui.h>
#include <src/winui/WinAppTrayModule.h>

#define KCC_FILE "WinAppTray"

namespace kcc
{
    // explorer command
    static const String k_ie("explorer ");

    // singleton (GOF)
    WinAppTray* WinAppTray::k_instance = NULL;

    // ctor/dtor
    WinAppTray::WinAppTray() : m_module(new WinAppTrayModule(this)) 
    {
        KCC_ASSERT(k_instance == NULL, KCC_FILE, "WinAppTray", "Only 1 instance of app allowed");
        k_instance = this;
    }
    WinAppTray::~WinAppTray() {}

    // instance: accessor to application singleton
    WinAppTray& WinAppTray::instance() 
    { 
        KCC_ASSERT(k_instance != NULL, KCC_FILE, "instance", "App not created yet");
        return *k_instance; 
    }

    // trayRun: begin running application
    int WinAppTray::trayRun(HINSTANCE h)
    {
        // set resources
        _AtlBaseModule.SetResourceInstance(h); 

        // parse path and command line, initialize gps, and run app
        parseCommandLine();
        Properties props;
        onTraySetup(props, m_config);
        props.set("gps.systemDir", m_path);
        kcc::init(props);

        // only run 1 instance
        String appName(gps::properties().get("Log.name", "WinAppTray"));
        m_mutex = ::CreateMutex(NULL, TRUE, appName.c_str());
        if (m_mutex == NULL || GetLastError()==ERROR_ALREADY_EXISTS) return 1;
        return m_module->WinMain(SW_SHOWNORMAL); 
    }

    // trayExit: attempt to exit application
    void WinAppTray::trayExit() 
    { 
        if (!onTrayExit(false)) return;
        m_module->window().DestroyWindow();
        ::CloseHandle(m_mutex);
        ::PostQuitMessage(0);
    }

    // trayHWND: get tray hwnd
    HWND WinAppTray::trayHWND() { return m_module->window(); }

    // messageOk: ok message box
    void WinAppTray::messageOk(const String& msg) 
    { 
        ::MessageBox(
            NULL, msg.c_str(), 
            gpsPlatformUtil::loadText(m_config.idResource).c_str(), 
            MB_OK|MB_ICONINFORMATION); 
    }

    // messageErr: error message box
    void WinAppTray::messageErr(const String& msg) 
    { 
        ::MessageBox(
            NULL, msg.c_str(), 
            gpsPlatformUtil::loadText(m_config.idResource).c_str(), 
            MB_OK|MB_ICONERROR); 
    }

    // messageAsk: prompt with ok/cancel
    bool WinAppTray::messageAsk(const String& msg)
    {
        return ::MessageBox(
            NULL, msg.c_str(), 
            gpsPlatformUtil::loadText(m_config.idResource).c_str(), 
            MB_OKCANCEL|MB_ICONQUESTION) == IDOK;
    }

    // explorer: run IE
    void WinAppTray::explorer(const String& url)
    {
        ::STARTUPINFO         si = {0};
        ::PROCESS_INFORMATION pi = {0};
        ::CreateProcess(NULL, (LPSTR)(k_ie + url).c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
    }

    // startTimer: start timer with id
    void WinAppTray::startTimer(int repeat, int id)
    {
        killTimer(id);
        int t = ::SetTimer(NULL, NULL, repeat, (TIMERPROC) WinAppTray::onTimer);
        m_timersById[id]   = t;
        m_timersByTimer[t] = id;
    }

    // killTimer: kill timer with id
    void WinAppTray::killTimer(int id)
    {
        Timers::iterator find = m_timersById.find(id);
        if (find != m_timersById.end()) 
        {
            int t = find->second;
            ::KillTimer(NULL, t);
            m_timersById.erase(id);
            m_timersByTimer.erase(t);
        }
    }

    // argValue: query argument value
    bool WinAppTray::argValue(const String& arg, String& value)
    {
        for (StringVector::iterator i = m_args.begin(); i != m_args.end(); i++)
        {
            String::size_type find = i->find(arg);
            if (find != String::npos)
            {
                value = Strings::trim(i->substr(find+1));
                return true;
            }
        }
        return false;
    }

    // Window proxy methods
    void WinAppTray::trayAnimateStart()                { m_module->window().animateStart(); }
    void WinAppTray::trayAnimateStop()                 { m_module->window().animateStop(); }
    void WinAppTray::trayMenuEnable(USHORT id, bool e) { m_module->window().menuEnable(id, e); }
    void WinAppTray::trayOpen(const String& link)      { m_module->window().open(link); }
    void WinAppTray::trayClose()                       { m_module->window().close(); }

    // onTrayInit: intialize tray window
    bool WinAppTray::onTrayInit() { return m_module->window().init(m_config); }

    // onTrayCommand: default implementation for menu default and exit
    void WinAppTray::onTrayCommand(int cmd)
    {
        if      (cmd == m_config.idMenuDefault) trayOpen(m_config.urlDefault);
        else if (cmd == m_config.idMenuExit)    trayExit();
    }

    // No-op implementations
    bool WinAppTray::onTrayLink(const String&, const String&, const String&) { return false; }
    void WinAppTray::onTrayMenuPopup() {}

    // parseCommandLine: parse command line into local structures
    void WinAppTray::parseCommandLine()
    {
        String mp(::GetCommandLine());

        // split line on '"' boundaries
        StringVector tokens;
        Strings::tokenize(mp, "\"", tokens);
        mp.assign(Strings::trim(tokens[0]));

        // command line param's
        for (StringVector::size_type i = 1; i < tokens.size(); i++) 
            m_args.push_back(Strings::trim(tokens[i]));

        // trim application name from path
        String::size_type n = mp.rfind("\\");
        if (n == String::npos)
        {
            char buf[512];
            ::GetCurrentDirectory(sizeof(buf), buf);
            mp.assign(buf);
        }
        else
        {
            mp.erase(n);
        }

        // replace '\' with '/'
        Strings::tokenize(mp, "\\", tokens);
        Strings::join(Platform::sep(), tokens.begin(), tokens.end(), m_path);
        m_path.append(Platform::sep());
    }

    // onTimer: timer callback, delegate to command
    void WinAppTray::onTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
    {
        WinAppTray& app = WinAppTray::instance();
        app.onTrayCommand(app.m_timersByTimer[idEvent]);
    }
}

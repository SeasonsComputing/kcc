/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTrayModule.h,v 1.1 2003/11/25 05:31:35 tedk Exp $
 */
#ifndef WinAppTrayModule_h
#define WinAppTrayModule_h

#include <src/winui/WinAppTrayWindow.h>

/**
 * Implementation of tray app module
 *
 * @author Ted V. Kremer
 */
class WinAppTrayModule : public ATL::CAtlExeModuleT<WinAppTrayModule>
{
public:
    /** 
     * Ctor/dtor
     * @param handler handler callback
     */
    WinAppTrayModule(IWinAppTrayHandler* handler);
    virtual ~WinAppTrayModule();

    // Accessors
    WinAppTrayWindow& window() { return m_window; } 

    // Implementation
    virtual HRESULT PreMessageLoop(int nShowCmd);

protected:
    // Attributes
    IWinAppTrayHandler* m_handler;
    WinAppTrayWindow    m_window;

private:
    WinAppTrayModule(const WinAppTrayModule&);
    WinAppTrayModule& operator = (const WinAppTrayModule&);
};

#endif // WinAppTrayModule_h

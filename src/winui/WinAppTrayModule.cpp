/*
 * Kuumba C++ Core
 *
 * $Id: WinAppTrayModule.cpp,v 1.2 2003/11/26 18:17:12 tedk Exp $
 */
#include <inc/core/Core.h>
#include <inc/winui/IWinAppTrayHandler.h>
#include <src/winui/winui.h>
#include <src/winui/WinAppTrayModule.h>

#define KCC_FILE "WinAppTrayModule"

namespace kcc
{
    // ctor/dtor
    WinAppTrayModule::WinAppTrayModule(IWinAppTrayHandler* handler) : m_handler(handler), m_window(handler) {}
    WinAppTrayModule::~WinAppTrayModule() {}

    // PreMessageLoop: intialize window
    HRESULT WinAppTrayModule::PreMessageLoop(int nShowCmd)
    {
        if (FAILED(ATL::CAtlExeModuleT<WinAppTrayModule>::PreMessageLoop(nShowCmd))) return S_FALSE;
        bool ok = m_handler->onTrayInit();
        if (ok) m_window.loaded();
        else    m_window.DestroyWindow();
        return ok ? S_OK : S_FALSE;
    }
}

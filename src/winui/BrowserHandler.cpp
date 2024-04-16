/*
 * Kuumba C++ Core
 *
 * $Id: BrowserHandler.cpp,v 1.3 2003/12/29 04:44:33 tedk Exp $
 */
#include <inc/core/Core.h>
#include <src/winui/winui.h>
#include <src/winui/BrowserHandler.h>

#define KCC_FILE "BrowserHandler"

namespace kcc
{
    ATL::_ATL_FUNC_INFO BrowserHandler::k_beforeNavigate2 =
    { 
        CC_STDCALL, VT_EMPTY, 7,
        { 
            VT_DISPATCH, VT_VARIANT|VT_BYREF, VT_VARIANT|VT_BYREF,
            VT_VARIANT|VT_BYREF, VT_VARIANT|VT_BYREF, VT_VARIANT|VT_BYREF,
            VT_BOOL|VT_BYREF 
        }
    };

    // ctor/dtor
    BrowserHandler::BrowserHandler(IBrowserHandler* h) : m_handler(h), m_cookie(0) {}
    BrowserHandler::~BrowserHandler() { detach(); }

    // attach: attach self to browser event sink
    void BrowserHandler::attach(CWindow* parent)
    {
        ATL::CComPtr<IUnknown> unknown;
        ATL::CAxWindow ax(*parent);
        ax.CreateControlEx(L"about:blank", NULL, NULL, &unknown);
        m_browser = unknown;
        ATL::AtlAdvise(m_browser, (IUnknown*)this, DIID_DWebBrowserEvents2, &m_cookie);
    }

    // attach: detach self from browser event sink
    void BrowserHandler::detach()
    {
        if (m_cookie == 0) return; // already shutdown
        ATL::AtlUnadvise(m_browser, DIID_DWebBrowserEvents2, m_cookie);
        m_cookie = 0;
        m_browser.Release();
    }

    // navigate: navigate to URL
    void BrowserHandler::navigate(const String& url)
    {
        ATL::CComVariant v;
        m_browser->Navigate(ATL::CComBSTR(url.c_str()), &v, &v, &v, &v);
    }

    // onBeforeNavigate2: delegate event to handler
    void __stdcall BrowserHandler::onBeforeNavigate2(
        IDispatch* disp, VARIANT* url, VARIANT* flags, VARIANT* frame, 
        VARIANT* post, VARIANT* hdrs, VARIANT_BOOL* cancel)
    {
        // convert com data to strings and delegate
        String strUrl(Strings::printf("%ls", url->bstrVal));
        String strFrame((frame->bstrVal==NULL) ? "" : Strings::printf("%ls", frame->bstrVal));
        String strPost;
        strPost.reserve(1024);
        if (post->pvarVal->parray != NULL)
        {
            ATL::CComSafeArray<char> psa(post->pvarVal->parray);
            for (ULONG i = 0; i < psa.GetCount(); i++) strPost += psa.GetAt(i);
        }
        String redirect;
        if (m_handler->onBrowserLink(strUrl, strFrame, Strings::httpDecode(strPost))) *cancel = TRUE;
    }
}

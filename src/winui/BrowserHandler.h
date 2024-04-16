/*
 * Kuumba C++ Core
 *
 * $Id: BrowserHandler.h,v 1.1 2003/11/25 05:31:35 tedk Exp $
 */
#ifndef BrowserHandler_h
#define BrowserHandler_h

#include <src/winui/IBrowserHandler.h>

namespace kcc
{
    /**
     * Halper class to intercept browser events and delegate to handler
     *
     * @author Ted V. Kremer
     */
    class BrowserHandler : public IDispEventSimpleImpl<5150, BrowserHandler, &DIID_DWebBrowserEvents2>
    {
    public:
        /**
         * Ctor/dtor
         * @param h handler to receive delegated browser events
         */
        BrowserHandler(IBrowserHandler* h);
        virtual ~BrowserHandler();

        /**
         * Attach to browser (start receiving events from browser)
         * @param browser browser to attach to 
         */
        virtual void attach(CWindow* parent);

        /**
         * Detach from browser (stop receiving events from browser)
         * @param browser browser to attach to 
         */
        virtual void detach();

        /**
         * Navigate to URL
         * @param url to navigate to 
         */
        virtual void navigate(const String& url);

        /** Browser callback sink */
        BEGIN_SINK_MAP(BrowserHandler)
            SINK_ENTRY_INFO(5150, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, 
                            onBeforeNavigate2, &k_beforeNavigate2)
        END_SINK_MAP()

    protected:
        // Attributes
        CComQIPtr<IWebBrowser2> m_browser;
        DWORD                   m_cookie;
        IBrowserHandler*        m_handler;

        // Browser callbacks
        static _ATL_FUNC_INFO k_beforeNavigate2;
        void __stdcall onBeforeNavigate2(IDispatch* disp, VARIANT* url, VARIANT* flags, VARIANT* frame, 
                                        VARIANT* post, VARIANT* hdrs, VARIANT_BOOL* cancel);

    private:
        BrowserHandler(const BrowserHandler&);
        BrowserHandler& operator = (const BrowserHandler&);
    };
}

#endif // BrowserHandler_h

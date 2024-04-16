/*
 * Kuumba C++ Core
 *
 * $Id: TaskDialog.cpp,v 1.6 2004/01/06 23:12:45 tedk Exp $
 */
#include <inc/core/Core.h>
#include <inc/winui/TaskDialog.h>
#include <src/winui/winui.h>

#define KCC_FILE "TaskDialog"

namespace kcc
{
    // dialog layout
    static const int k_dlgPad = 0;
    static const int k_dlgRow = 17;

    //
    // TaskDialogImpl Implementation
    //

    // Implementation class for dialog UI
    class TaskDialogImpl : public CAxDialogImpl<TaskDialogImpl>
    {
    public:
        // ctor/dtor
        TaskDialogImpl(
            USHORT id, USHORT idHeader, USHORT idProgress, 
            SHORT idStatus, USHORT idCancel,
            COLORREF bg)  
            : IDD(id), m_idHeader(idHeader), m_idProgress(idProgress), m_idStatus(idStatus), 
            m_idCancel(idCancel), m_activity(NULL), m_interrupted(false)
        {
            INITCOMMONCONTROLSEX cc = {0};
            cc.dwICC = ICC_PROGRESS_CLASS ;
            ::InitCommonControlsEx(&cc);
            m_bg = ::CreateSolidBrush(bg); 
        }
        virtual ~TaskDialogImpl() { ::DeleteObject(m_bg); }

        // steps: set steps for activity
        void steps(int steps) { m_progress.SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, steps)); }

        // status: set stus message and progress
        void status(const String& msg) 
        { 
            m_progress.SendMessage(PBM_STEPIT, 0, 0); 
            m_status.SetWindowText(msg.c_str());
        }

        // interrupted: query if activity was interrupted
        bool interrupted() { return m_interrupted; }

        // exit: exit dialog
        void exit()
        {
            m_activity = NULL;
            EndDialog(0);
        }

    protected:
        // Implementation
        BEGIN_MSG_MAP(TaskDialogImpl)
            MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
            COMMAND_ID_HANDLER(IDCANCEL, onCancel)
            MESSAGE_HANDLER(WM_ERASEBKGND, onEraseBg)
            MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onEraseLabelBg)
        END_MSG_MAP()

        // onInitDialog: run thread task
        LRESULT onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            KCC_ASSERT(m_activity != NULL, KCC_FILE, "TaskDialogImpl", "Attached activity is NULL");

            // get dlg components
            HBITMAP      bmp = ::LoadBitmap(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(m_idHeader));
            ATL::CWindow hdr = GetDlgItem(m_idHeader);
            m_progress = GetDlgItem(m_idProgress);
            m_status   = GetDlgItem(m_idStatus);
            KCC_ASSERT(
                bmp != NULL && m_progress.m_hWnd != NULL && m_status.m_hWnd != NULL && hdr.m_hWnd != NULL, 
                KCC_FILE, "TaskDialogImpl", "Unable to locate task dialog resource");
            ATL::CComBSTR txt;
            m_status.GetWindowText(&txt);
            m_default.assign(CW2A(txt));

            // resize window and contents around header
            BITMAP bmpData = {0};
            ::GetObject(bmp, sizeof(BITMAP), &bmpData);
            ATL::CSize sz(bmpData.bmWidth, bmpData.bmHeight);
            ::DeleteObject(bmp);
            ::ResizeClient(sz.cx+(k_dlgPad*2), sz.cy+(k_dlgRow*4)+(k_dlgPad*2), FALSE);
            ::CenterWindow(::GetDesktopWindow());

            int x = k_dlgPad;
            int y = k_dlgPad;
            hdr.MoveWindow(x, y, sz.cx, sz.cy, FALSE);            
            y += sz.cy;

            int inset = sz.cx*1/6;
            m_progress.MoveWindow(x+inset, y, sz.cx-(inset*2), k_dlgRow, FALSE); 
            y += k_dlgRow+k_dlgRow;

            m_status.MoveWindow(x, y, sz.cx, k_dlgRow, FALSE);   

            // start activity
            m_progress.SendMessage(PBM_SETSTEP, (WPARAM)1, 0); 
            m_activity->go();
            return 1;
        }

        // onCancel: interrupt thread
        LRESULT onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
        {
            if (::MessageBox(
                m_hWnd, 
                gpsPlatformUtil::loadText(m_idCancel).c_str(),
                gpsPlatformUtil::loadText(IDD).c_str(),
                MB_OKCANCEL|MB_ICONQUESTION) == IDOK) 
            {
                // activity may complete while dialog is up so check for null activity
                if (m_activity != NULL) m_activity->interrupt();
                m_interrupted = true;
                status(m_default);
            }
            return 0;
        }

        // onEraseBg: paint background of dialog
        LRESULT onEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            ::PAINTSTRUCT p = {0};
            ::BeginPaint(&p);
            ::FillRect(p.hdc, &p.rcPaint, m_bg);
            ::EndPaint(&p);
            return 1;
        }

        // onEraseLabelBg: paint background of label
        LRESULT onEraseLabelBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            ::SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)m_bg;
        }

        // Attributes
        USHORT              IDD;
        USHORT              m_idHeader;
        USHORT              m_idProgress;
        USHORT              m_idStatus;
        USHORT              m_idCancel;
        ATL::CWindow        m_progress;
        ATL::CWindow        m_status;
        TaskDialogActivity* m_activity;
        bool                m_interrupted;
        String              m_default;
        HBRUSH              m_bg;
    };

    //
    // TaskDialogActivity Implementation
    //

    // ctor/dtor
    TaskDialogActivity::TaskDialogActivity(TaskDialog* dlg, const Char* name)
        : gpsThread(name, dlg), m_dialog(NULL), m_interrupted(false), m_steps(0)
    {
        if (dlg != NULL) 
        {
            m_dialog = dlg->m_dialog;
            m_dialog->m_activity = this; // attach to dialog
        }
    }

    // interrupt: interrupt activity
    void TaskDialogActivity::interrupt() 
    { 
        Mutex::Lock lock(m_sentinel); 
        m_interrupted = true; 
    }

    // interrupted: query if interrupted
    bool TaskDialogActivity::interrupted() 
    { 
        Mutex::Lock lock(m_sentinel); 
        return m_interrupted;
    }

    // steps: set steps for activity
    void TaskDialogActivity::steps(int steps) { m_steps = steps; if (m_dialog != NULL) m_dialog->steps(steps); }
    int TaskDialogActivity::steps()           { return m_steps; }

    // status: write status to dialog and log
    void TaskDialogActivity::status(const String& msg)
    {
        Log::Scope scope(name(), (name()==NULL) ? "status" : name());
        Log::info(msg.c_str());
        if (m_dialog != NULL) m_dialog->status(msg);
    }


    //
    // TaskDialog Implementation
    //

    // ctor/dtor
    TaskDialog::TaskDialog(
        USHORT id, USHORT idHeader, USHORT idProgress, 
        USHORT idStatus, USHORT idCancel,
        COLORREF bg)
        : m_dialog(new TaskDialogImpl(id, idHeader, idProgress, idStatus, idCancel, bg))
    {}
    TaskDialog::~TaskDialog() {}

    // runTask: show dialog 
    bool TaskDialog::runTask(HWND parent) 
    { 
        m_dialog->DoModal(parent); 
        return !m_dialog->interrupted(); 
    }

    // onThreadDeleted: destroy dialog when activity completes
    void TaskDialog::onThreadDeleted(const Char* name) { m_dialog->exit(); }
}

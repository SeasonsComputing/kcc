/*
 * Kuumba C++ Core
 *
 * $Id: TaskDialog.h,v 1.3 2004/01/06 23:13:24 tedk Exp $
 */
#ifndef TaskDialog_h
#define TaskDialog_h

namespace kcc
{
    class TaskDialog;
    class TaskDialogImpl;
    class TaskDialogActivity;

    /**
     * Thread task that may optionally be attached to a status dialog
     *
     * Usage:
     *   struct MyTask : TaskDialogActivity
     *   {
     *     MyTask(Data* data) : TaskDialogActivity(null, "MyTask") {}
     *     MyTask(TaskDialogActivity* dlg, Data* data) : TaskDialogActivity(dlg, "MyTask") {}
     *     void invoke()
     *     {
     *       steps(3);
     *
     *       status("Started...");
     *       ...do activity
     *       if (interrupted()) return;
     *       
     *       status("Next activity...");
     *       ...do activity
     *       if (interrupted()) return;
     *
     *       ...
     *     }
     *   }
     *
     * @author Ted V. Kremer
     */
    class TaskDialogActivity : public IThread
    {
    public:
        /**
         * Inerrupt task - thread safe
         */
        void interrupt();
        bool interrupted();

    protected:
        /**
         * Create dialog task
         * @param dlg OPTIONAL when set status is sent to dlg otherwise 
         *            status is set as log info data
         * @param name thread name
         */
        TaskDialogActivity(TaskDialog* dlg, const Char* name = NULL);

        /**
         * Set total number of steps
         * @param steps number of steps
         */
        void steps(int steps);
        int  steps();

        /**
         * Update status of tasks
         * @param msg text of status
         */
        void status(const String& msg);

    private:
        // Attributes
        Mutex           m_sentinel;
        int             m_steps;
        bool            m_interrupted;
        TaskDialogImpl* m_dialog;

        TaskDialogActivity(const TaskDialogActivity&);
        TaskDialogActivity& operator = (const TaskDialogActivity&);
    };

    /**
     * Task dialog to show while task is running (MODAL)
     *
     * Usage:
     *   TaskDialog d(ID_DLG, ID_DLG_STATUS);
     *   MyTask* t = new MyTask(&d);
     *   d.runTask(); // blocking call
     *
     * @author Ted V. Kremer
     */
    class TaskDialog : public gpsIThreadManager
    {
    public:
        /**
         * Ctor/dtor
         * @param id id of dialog resource
         * @param idHeader id of bitmap and bitmap control
         * @param idProgress id of progress control
         * @param idStatus id of status label
         * @param isCancel id of cancel message
         * @param steps number of steps in acitivity
         * @param bg background color of dialog
         */
        TaskDialog(
            USHORT id, USHORT idHeader, 
            USHORT idProgress, USHORT isStatus, USHORT idCancel,
            COLORREF bg);
        virtual ~TaskDialog();

        /**
         * Run the thread dialog task and show dialog
         * @return true if completed fully, false if interrupted
         */
        bool runTask(HWND parent=NULL);

    private:
        // Implementation
        void onThreadDeleted(const Char* name); // will close dialog when task completes

        // Attributes
        friend TaskDialogActivity;
        AutoPtr<TaskDialogImpl> m_dialog;

        TaskDialog(const TaskDialog&);
        TaskDialog& operator = (const TaskDialog&);
    };
}

#endif // gpsThreadDialog_h

// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ProcessManager.h"

class CMainDlg : 
	public CDialogImpl<CMainDlg>,
	public CDoubleBufferImpl<CMainDlg>,
	public CAutoUpdateUI<CMainDlg>,
	public CCustomDraw<CMainDlg>,
	public CMessageFilter,
	public CIdleHandler {
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void DoPaint(CDCHandle dc);
	DWORD OnPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnItemPrePaint(int, LPNMCUSTOMDRAW cd);

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(ID_TOGGLE_KERNEL, OnToggleKernel)
		COMMAND_ID_HANDLER(ID_RUN, OnToggleRun)
		CHAIN_MSG_MAP(CDoubleBufferImpl<CMainDlg>)
		CHAIN_MSG_MAP(CCustomDraw<CMainDlg>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnToggleKernel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnToggleRun(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

	int m_ShowCount{ 10 };
	ProcessManager m_ProcMgr;
	CBrush m_CpuBrush, m_KernelBrush;
	CToolBarCtrl m_ToolBar;
	int m_LineHeight = 20;
	int m_Interval{ 750 };
	bool m_ShowKernelTimes{ false };
	bool m_Run{ true };
};

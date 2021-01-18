// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) {
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle() {
	UIUpdateChildWindows();
	return FALSE;
}

void CMainDlg::DoPaint(CDCHandle dc) {
	CRect rc;
	GetClientRect(&rc);
	m_ToolBar.GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.left += 4;
	rc.top = rc.bottom + 4;

	dc.SetBkMode(TRANSPARENT);
	dc.SetBkColor(0);
	dc.SetTextColor(RGB(255, 255, 255));
	dc.SelectFont(GetFont());

	CString text;
	auto& processes = m_ProcMgr.GetProcesses();
	auto count = std::min((int)processes.size(), m_ShowCount);

	for (int i = 0; i < count; i++) {
		auto& pi = processes[i];
		CRect pidRect(rc.left, rc.top + i * m_LineHeight, rc.left + 50, rc.top + (i + 1) * m_LineHeight);
		text.Format(L"%u", pi.Pid);
		dc.DrawText(text, text.GetLength(), &pidRect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
		CRect nameRect(pidRect);
		nameRect.OffsetRect(60, 0);
		nameRect.right = nameRect.left + 180;
		dc.DrawText(pi.Name, pi.Name.GetLength(), &nameRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

		nameRect.left = nameRect.right + 10;
		if (m_ShowKernelTimes) {
			nameRect.right = nameRect.left + int(pi.KernelCPU * 3 + .5);
			nameRect.DeflateRect(0, 2);
			dc.FillRect(nameRect, m_KernelBrush);
			nameRect.left = nameRect.right;
			nameRect.right = nameRect.left + int((pi.CPU - pi.KernelCPU) * 3 + .5);
			dc.FillRect(nameRect, m_CpuBrush);
		}
		else {
			nameRect.right = nameRect.left + int(pi.CPU * 3 + .5);
			nameRect.DeflateRect(0, 2);
			dc.FillRect(nameRect, m_CpuBrush);
		}

		nameRect.left = nameRect.right + 10;
		nameRect.InflateRect(0, 2);
		nameRect.right = nameRect.left + 50;
		text.Format(L"%.2f%%", pi.CPU);
		dc.DrawText(text, text.GetLength(), &nameRect, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
	}
}

DWORD CMainDlg::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	if (cd->hdr.hwndFrom == m_ToolBar) {
		CDCHandle dc(cd->hdc);
		dc.FillRect(&cd->rc, (HBRUSH)::GetStockObject(BLACK_BRUSH));
		return CDRF_NOTIFYITEMDRAW;
	}
	return CDRF_DODEFAULT;
}

DWORD CMainDlg::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto tb = (LPNMTBCUSTOMDRAW)cd;
	//tb->clrBtnFace = RGB(0, 0, 0);
	tb->clrHighlightHotTrack = RGB(0, 0, 255);
	return TBCDRF_USECDCOLORS | TBCDRF_HILITEHOTTRACK;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// center the dialog on the screen
	CenterWindow();

	CString text;
	text.LoadString(IDR_MAINFRAME);
	SetWindowText(text);
	CRect rc;
	GetDlgItem(IDC_PAINT).GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.right = rc.left + 700;
	rc.bottom = rc.top + m_LineHeight * (m_ShowCount + 0) + ::GetSystemMetrics(SM_CYCAPTION);
	SetWindowPos(nullptr, 0, 0, rc.Width(), rc.top * 2 + rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	auto hToolBar = m_ToolBar.Create(*this, rcDefault, nullptr, ATL_SIMPLE_TOOLBAR_STYLE | TBSTYLE_FLAT, 0, ATL_IDW_TOOLBAR);
	m_ToolBar.SetIndent(4);

	int size = 24;
	CImageList images;
	images.Create(size, size, ILC_COLOR32, 4, 4);
	m_ToolBar.SetImageList(images);
	m_ToolBar.MoveWindow(0, 0, 200, 32);

	const struct {
		UINT id;
		int image;
		BYTE style = BTNS_BUTTON;
		BYTE state = TBSTATE_ENABLED;
		PCWSTR text = nullptr;
	} buttons[] = {
		{ ID_RUN, IDI_PLAY, BTNS_CHECK, TBSTATE_CHECKED | TBSTATE_ENABLED },
		{ 0 },
		{ ID_TOGGLE_KERNEL, IDI_ATOM, BTNS_CHECK },
	};

	for (auto& b : buttons) {
		if (b.id == 0)
			m_ToolBar.AddSeparator(4);
		else {
			int image = images.AddIcon(AtlLoadIconImage(b.image, 0, size, size));
			m_ToolBar.AddButton(b.id, b.style, b.state, image, b.text, 0);
		}
	}

	UIAddToolBar(hToolBar);

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	m_CpuBrush.CreateSolidBrush(RGB(0, 0, 255));
	m_KernelBrush.CreateSolidBrush(RGB(255, 0, 0));

	SetTimer(1, m_Interval, nullptr);

	return TRUE;
}

LRESULT CMainDlg::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && !IsIconic()) {
		m_ProcMgr.Update();
		Invalidate(FALSE);
	}
	return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnToggleKernel(WORD, WORD wID, HWND, BOOL&) {
	m_ShowKernelTimes = !m_ShowKernelTimes;
	UISetCheck(ID_TOGGLE_KERNEL, m_ShowKernelTimes);
	return 0;
}

LRESULT CMainDlg::OnToggleRun(WORD, WORD wID, HWND, BOOL&) {
	m_Run = !m_Run;
	if (m_Run)
		SetTimer(1, m_Interval, nullptr);
	else
		KillTimer(1);
	UISetCheck(ID_RUN, m_Run);
	return 0;
}

void CMainDlg::CloseDialog(int nVal) {
	DestroyWindow();
	::PostQuitMessage(nVal);
}


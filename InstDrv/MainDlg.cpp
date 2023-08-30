// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include "SysManager.h"

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	DragAcceptFiles();
	ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_hWnd, WM_COPYGLOBALDATA, MSGFLT_ALLOW, nullptr);

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::ShellAbout(m_hWnd, L"Install Driver", L"This sample program is used to load kernel driver.",
		AtlLoadIcon(IDR_MAINFRAME));
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnDropFile(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	auto hDrop = reinterpret_cast<HDROP>(wParam);
	CString filePath;
	filePath.Preallocate(MAX_PATH);
	DragQueryFile(hDrop, 0, filePath.GetBuffer(), MAX_PATH);
	GetDlgItem(IDC_EDIT_DRVFILEPATH).SetWindowText(filePath);
	return TRUE;
}

LRESULT CMainDlg::OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CSimpleFileDialog dlg(TRUE, L".sys", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER,
		L"Sys File (*.sys)\0*.sys\0All Files (*.*)\0*.*\0", *this);
	if (dlg.DoModal() == IDOK) {
		SetDlgItemText(IDC_EDIT_DRVFILEPATH, dlg.m_szFileName);
	}
	return 0;
}

bool CMainDlg::InitParams(CString& sysPath, CString& serviceName, CString& displayName, 
	bool& isFileSys) {
	GetDlgItemText(IDC_EDIT_DRVFILEPATH, sysPath);
	if (sysPath.IsEmpty()) {
		SetDlgItemText(IDC_EDIT_STATUS, L"请注意，文件路径为空！");
		return false;
	}
	serviceName = _tcsrchr(sysPath, L'\\') + 1;
	if (serviceName.Right(4).CompareNoCase(L".sys") == 0) {
		serviceName.Delete(serviceName.GetLength() - 4, 4);
	}
	displayName = serviceName;

	UINT bst = IsDlgButtonChecked(IDC_DRIVER_TYPE);
	switch (bst)
	{
	case BST_UNCHECKED:
		isFileSys = false;
		break;

	case BST_CHECKED:
		isFileSys = true;
		break;

	default:
		break;
	}
	return true;
}

void CMainDlg::ShowError() {
	DWORD err = GetLastError();
	DWORD sysLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	wil::unique_hlocal hMem;
	bool fOk = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
		| FORMAT_MESSAGE_ALLOCATE_BUFFER,
		nullptr, err, sysLocale, (PTSTR)hMem.addressof(), 0, nullptr);

	if (!fOk) {
		wil::unique_hmodule hDll;
		hDll.reset(LoadLibraryEx(L"netmsg.dll", NULL, DONT_RESOLVE_DLL_REFERENCES));
		if (hDll.get() != NULL) {
			fOk = FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				hDll.get(), err, sysLocale, (PTSTR)hMem.addressof(), 0, nullptr);
		}
	}

	if (fOk) {
		ATLASSERT(hMem.get());
		SetDlgItemText(IDC_EDIT_STATUS, (LPCTSTR)LocalLock(hMem.get()));
	}
	else {
		SetDlgItemText(IDC_EDIT_STATUS, L"没找到这个错误的相关描述");
	}
	return;
}

LRESULT CMainDlg::OnInstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CString sysPath, serviceName, displayName;
	bool isFileSys = false;
	bool success = InitParams(sysPath, serviceName, displayName, isFileSys);
	if (!success)
		return FALSE;
	SysManager SysMgr(sysPath, serviceName, displayName, isFileSys);
	success = SysMgr.Install();
	if (!success) {
		ShowError();
		return FALSE;
	}
	SetDlgItemText(IDC_EDIT_STATUS, L"成功安装服务!");
	return TRUE;
}

LRESULT CMainDlg::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CString sysPath, serviceName, displayName;
	bool isFileSys = false;
	bool success = InitParams(sysPath, serviceName, displayName, isFileSys);
	if (!success)
		return FALSE;
	SysManager SysMgr(sysPath, serviceName, displayName, isFileSys);
	success = SysMgr.Remove();
	if (!success) {
		ShowError();
		return FALSE;
	}
	SetDlgItemText(IDC_EDIT_STATUS, L"成功移除服务!");

	return TRUE;
}

DWORD CMainDlg::StopService() {
	CString sysPath, serviceName, displayName;
	bool isFileSys;
	bool success = InitParams(sysPath, serviceName, displayName, isFileSys);
	if (!success)
		return FALSE;
	SysManager SysMgr(sysPath, serviceName, displayName, isFileSys);
	success = SysMgr.Stop();
	if (!success) {
		ShowError();
		return FALSE;
	}
	SetDlgItemText(IDC_EDIT_STATUS, L"成功停止服务!");
	return TRUE;
}

LRESULT CMainDlg::OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	wil::unique_handle hThread;
	hThread.reset(::CreateThread(nullptr, 0, [](auto param) {
		return ((CMainDlg*)param)->StopService();
		}, this, 0, nullptr));
	return TRUE;
}

DWORD CMainDlg::RunService() {
	CString sysPath, serviceName, displayName;
	bool isFileSys = false;
	bool success = InitParams(sysPath, serviceName, displayName, isFileSys);
	if (!success)
		return FALSE;
	bool bypassRevokeCheck = false;
	UINT bst = IsDlgButtonChecked(IDC_BYPASS_REVOKE);
	switch (bst)
	{
		case BST_UNCHECKED:
			bypassRevokeCheck = false;
			break;

		case BST_CHECKED:
			bypassRevokeCheck = true;
			break;
	}
	if (bypassRevokeCheck) {
		MessageBox(L"Not implementation yet!", L"Info", MB_OK);
	}
	SysManager SysMgr(sysPath, serviceName, displayName, isFileSys);
	success = SysMgr.Run();
	if (!success) {
		ShowError();
		return FALSE;
	}
	SetDlgItemText(IDC_EDIT_STATUS, L"成功运行服务!");
	return TRUE;
}

LRESULT CMainDlg::OnRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	wil::unique_handle hThread;
	hThread.reset(::CreateThread(nullptr, 0, [](auto param) {
		return ((CMainDlg*)param)->RunService();
		}, this, 0, nullptr));
	return TRUE;
}
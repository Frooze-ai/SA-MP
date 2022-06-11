
// launch3Dlg.h : header file
//

#pragma once


// CLaunch3Dlg dialog
class CLaunch3Dlg : public CDialog
{
// Construction
public:
	CLaunch3Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LAUNCH3_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeNick();
	afx_msg void OnLaunch();
	afx_msg void OnButton2();
	afx_msg void OnButton1();
	DECLARE_MESSAGE_MAP()
};

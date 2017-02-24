// Copyright (c) CEZEO software Ltd. http://www.cezeo.com
// NeuroLogDlg.h : header file

#pragma once

#include "afxwin.h"

// CNeuroLogDlg dialog
class CNeuroLogDlg : public CDialog
{
// Construction
public:
	CNeuroLogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NEUROLOG_DIALOG };


	void UnlockDlg( BOOL unlockValue );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support



// Implementation
protected:
	HICON m_hIcon;

	LPSTR UNICODEtoASCII( CString* pUnicodeSrt );
	BOOL PickTheFolder( CWnd* lp_mfcWnd, CString *lp_csFolder );
	void SaveToRegsitry();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStart();

	CListBox logListBoxCtrl;

	uint32 hitsLimit;
	uint32 sizeLimit;

	CString logsMask;
	CString subnetsDbFolder;
	CString logsFolder;
	CString outputFolder;
	afx_msg void OnClickedButtonSubnets();
	afx_msg void OnClickedButtonLogs();
	afx_msg void OnClickedButtonOutput();
};

// Copyright (c) CEZEO software Ltd. http://www.cezeo.com
// NeuroLogDlg.h : header file

#pragma once

#include "afxeditbrowsectrl.h"
#include "afxwin.h"


// CNeuroLogDlg dialog
class CNeuroLogDlg : public CDialogEx
{
// Construction
public:
	CNeuroLogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NEUROLOG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	void SaveToRegsitry();
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStart();
	CMFCEditBrowseCtrl subnetFolderControl;
	CMFCEditBrowseCtrl logFolderControl;
	CMFCEditBrowseCtrl cacheFolderControl;
	CListBox logListBoxCtrl;
	CString logsMask;
	DWORD hitsLimit;
	DWORD sizeLimit;
	CString subnetsDbFolder;
	CString logsFolder;
	CString cacheFolder;
};

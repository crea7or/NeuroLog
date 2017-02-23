// NeuroLogDlg.cpp : implementation file
// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "NeuroLog.h"
#include "NeuroLogDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region About Dialog

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
#pragma endregion

// CNeuroLogDlg message handlers
#pragma region Main Dialog

// CNeuroLogDlg dialog
CNeuroLogDlg::CNeuroLogDlg(CWnd* pParent /*=NULL*/)	: CDialogEx(CNeuroLogDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	logsMask = GetRegistry()->GetProfileStringW( _T( "Paths" ), _T( "logsMask" ), _T( "" ) );

	hitsLimit = GetRegistry()->GetProfileIntW( _T( "Paths" ), _T( "hitsLimit" ), 500 );
	sizeLimit = GetRegistry()->GetProfileIntW( _T( "Paths" ), _T( "sizeLimit" ), 50000000 );
	subnetsDbFolder = GetRegistry()->GetProfileStringW( _T( "Paths" ), _T( "subnetsDbFolder" ), _T( "" ) );
	logsFolder = GetRegistry()->GetProfileStringW( _T( "Paths" ), _T( "logsFolder" ), _T( "" ) );
	cacheFolder = GetRegistry()->GetProfileStringW( _T( "Paths" ), _T( "cacheFolder" ), _T( "" ) );
}

void CNeuroLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_MFCEDITBROWSE1, subnetFolderControl );
	DDX_Control( pDX, IDC_MFCEDITBROWSE2, logFolderControl );
	DDX_Control( pDX, IDC_MFCEDITBROWSE3, cacheFolderControl );
	DDX_Control( pDX, IDC_LIST_LOG, logListBoxCtrl );
	DDX_Text( pDX, IDC_EDIT1, logsMask );
	DDX_Text( pDX, IDC_EDIT_HITS_LIMIT, hitsLimit );
	DDV_MinMaxUInt( pDX, hitsLimit, 0, 4000000000 );
	DDX_Text( pDX, IDC_EDIT_SIZE_LIMIT, sizeLimit );
	DDV_MinMaxUInt( pDX, sizeLimit, 0, 4000000000 );
	DDX_Text( pDX, IDC_MFCEDITBROWSE1, subnetsDbFolder );
	DDX_Text( pDX, IDC_MFCEDITBROWSE2, logsFolder );
	DDX_Text( pDX, IDC_MFCEDITBROWSE3, cacheFolder );
}

BEGIN_MESSAGE_MAP(CNeuroLogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNeuroLogDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNeuroLogDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON2, &CNeuroLogDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

BOOL CNeuroLogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set ListBox as Log destination
	GetCore()->appLog.listBoxCtrl = &logListBoxCtrl;

	subnetFolderControl.EnableFolderBrowseButton();
	logFolderControl.EnableFolderBrowseButton();
	cacheFolderControl.EnableFolderBrowseButton();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNeuroLogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CNeuroLogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CNeuroLogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNeuroLogDlg::OnBnClickedOk()
{
	// Stop using ListBox as log destination
	//GetCore()->appLog.listBoxCtrl = NULL;
	//CDialogEx::OnOK();
}

#pragma endregion


void CNeuroLogDlg::OnBnClickedCancel()
{
	SaveToRegsitry();

	// Stop using ListBox as log destination
	GetCore()->appLog.listBoxCtrl = NULL;
	CDialogEx::OnCancel();
}

void CNeuroLogDlg::SaveToRegsitry()
{
	UpdateData( TRUE );

	GetRegistry()->WriteProfileInt( _T( "Paths" ), _T( "hitsLimit" ), hitsLimit );
	GetRegistry()->WriteProfileInt( _T( "Paths" ), _T( "sizeLimit" ), sizeLimit );
	GetRegistry()->WriteProfileStringW( _T( "Paths" ), _T( "subnetsDbFolder" ), subnetsDbFolder );
	GetRegistry()->WriteProfileStringW( _T( "Paths" ), _T( "logsFolder" ), logsFolder );
	GetRegistry()->WriteProfileStringW( _T( "Paths" ), _T( "logsMask" ), logsMask );
	GetRegistry()->WriteProfileStringW( _T( "Paths" ), _T( "cacheFolder" ), cacheFolder );
}


void CNeuroLogDlg::OnBnClickedButtonStart()
{
	SaveToRegsitry();

	if ( subnetsDbFolder.IsEmpty() )
	{
		AfxMessageBox(_T("LOL, U Mad? Where are subnet files?"));
		return;
	}
	GetCore()->subnetsDbFolder = subnetsDbFolder;


	if ( logsFolder.IsEmpty() )
	{
		AfxMessageBox(_T("U Mad bro? Where are log files?"));
		return;
	}
	GetCore()->logsFolder = logsFolder;


	if ( logsMask.IsEmpty() )
	{
		AfxMessageBox( _T( "Where is logs mask?" ) );
		return;
	}
	GetCore()->logsMask = logsMask;


	cacheFolderControl.GetWindowText( cacheFolder );
	if ( cacheFolder.IsEmpty() )
	{
		AfxMessageBox(_T("Wtf, bro? We need a cache folder."));
		return;
	}
	GetCore()->cacheFolder = cacheFolder;

	GetCore()->hitsLimit = hitsLimit;
	GetCore()->sizeLimit = sizeLimit;

	GetCore()->ClearData();

	GetCore()->LoadSubnets();
	GetCore()->LoadLogs();
	GetCore()->AnalyzeSubnets();
}

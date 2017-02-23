// NeuroLogDlg.cpp : implementation file
// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "NeuroLog.h"
#include "NeuroLogDlg.h"
#include "Shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CNeuroLogDlg message handlers
#pragma region Main Dialog

// CNeuroLogDlg dialog
CNeuroLogDlg::CNeuroLogDlg(CWnd* pParent /*=NULL*/)	: CDialog(CNeuroLogDlg::IDD, pParent)
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
	CDialog::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_LIST_LOG, logListBoxCtrl );
	DDX_Text( pDX, IDC_EDIT_LOGS_MASK, logsMask );
	DDX_Text( pDX, IDC_EDIT_HITS_LIMIT, hitsLimit );
	DDV_MinMaxUInt( pDX, hitsLimit, 0, 4000000000 );
	DDX_Text( pDX, IDC_EDIT_SIZE_LIMIT, sizeLimit );
	DDV_MinMaxUInt( pDX, sizeLimit, 0, 4000000000 );
	DDX_Text( pDX, IDC_EDIT_SUBNETS_FOLDER, subnetsDbFolder );
	DDX_Text( pDX, IDC_EDIT_LOGS_FOLDER, logsFolder );
	DDX_Text( pDX, IDC_EDIT_CACHE_FOLDER, cacheFolder );
}

BEGIN_MESSAGE_MAP(CNeuroLogDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNeuroLogDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNeuroLogDlg::OnBnClickedCancel)
	ON_BN_CLICKED( IDC_BUTTON_START, &CNeuroLogDlg::OnBnClickedButtonStart )
	ON_BN_CLICKED( IDC_BUTTON_SUBNETS, &CNeuroLogDlg::OnClickedButtonSubnets )
	ON_BN_CLICKED( IDC_BUTTON_LOGS, &CNeuroLogDlg::OnClickedButtonLogs )
	ON_BN_CLICKED( IDC_BUTTON_CACHE, &CNeuroLogDlg::OnClickedButtonCache )
END_MESSAGE_MAP()

BOOL CNeuroLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set ListBox as Log destination
	GetCore()->appLog.listBoxCtrl = &logListBoxCtrl;

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	return TRUE;  // return TRUE  unless you set the focus to a control
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
		CDialog::OnPaint();
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
	//CDialog::OnOK();
}

#pragma endregion


void CNeuroLogDlg::OnBnClickedCancel()
{
	SaveToRegsitry();

	// Stop using ListBox as log destination
	GetCore()->appLog.listBoxCtrl = NULL;
	CDialog::OnCancel();
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


	if ( cacheFolder.IsEmpty() )
	{
		AfxMessageBox(_T("Wtf, bro? We need a cache folder."));
		return;
	}
	GetCore()->cacheFolder = cacheFolder;

	GetCore()->hitsLimit = hitsLimit;
	GetCore()->sizeLimit = sizeLimit;

	GetCore()->LoadSubnets();
	GetCore()->LoadLogs();
	GetCore()->AnalyzeSubnets();

	GetCore()->ClearData();
}

BOOL CNeuroLogDlg::PickTheFolder( CWnd* lp_mfcWnd, CString *lp_csFolder )
{
	BOOL l_boolResult = FALSE;

	LPMALLOC		pMalloc;
	BROWSEINFO		bi;
	LPITEMIDLIST	pidl;

	::ZeroMemory( &bi, sizeof( bi ) );
	CString l_csStartPath;
	l_csStartPath = _T( "C:\\" );
	TCHAR	m_szSelectedFolder[ MAX_PATH ];
	memset( m_szSelectedFolder, 0, sizeof( m_szSelectedFolder ) );


	// Gets the Shell's default allocator
	if ( ::SHGetMalloc( &pMalloc ) == NOERROR )
	{
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		if ( lp_mfcWnd != NULL )
		{
			bi.hwndOwner = lp_mfcWnd->m_hWnd;
		}
		bi.pidlRoot = NULL;
		bi.pszDisplayName = m_szSelectedFolder;
		bi.lpszTitle = l_csStartPath;
		bi.ulFlags = 0x40 | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_EDITBOX;
		bi.lpfn = NULL;
		bi.lParam = ( LPARAM )( LPCTSTR )l_csStartPath;
		// This next call issues the dialog box.
		if ( ( pidl = ::SHBrowseForFolder( &bi ) ) != NULL )
		{
			if ( ::SHGetPathFromIDList( pidl, m_szSelectedFolder ) )
			{
				// At this point pszBuffer contains the selected path
				lp_csFolder->Empty();
				*lp_csFolder = m_szSelectedFolder;
				l_boolResult = TRUE;
			} // if
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free( pidl );
		} // if
		// Release the shell's allocator.
		pMalloc->Release();
	} // if



	return l_boolResult;
}


void CNeuroLogDlg::OnClickedButtonSubnets()
{
	UpdateData( TRUE );
	PickTheFolder( this, &subnetsDbFolder );
	UpdateData( FALSE );
}


void CNeuroLogDlg::OnClickedButtonLogs()
{
	UpdateData( TRUE );
	PickTheFolder( this, &logsFolder );
	UpdateData( FALSE );
}


void CNeuroLogDlg::OnClickedButtonCache()
{
	UpdateData( TRUE );
	PickTheFolder( this, &cacheFolder );
	UpdateData( FALSE );
}

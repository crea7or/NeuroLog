// NeuroLogDlg.cpp : implementation file
// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "NeuroLog.h"
#include "NeuroLogDlg.h"
#include "Shlobj.h"
#include "ChronoTimer.h"

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
	outputFolder = GetRegistry()->GetProfileStringW( _T( "Paths" ), _T( "outputFolder" ), _T( "" ) );
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
	DDX_Text( pDX, IDC_EDIT_OUTPUT_FOLDER, outputFolder );
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
	ON_BN_CLICKED( IDC_BUTTON_OUTPUT, &CNeuroLogDlg::OnClickedButtonOutput )
END_MESSAGE_MAP()

BOOL CNeuroLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set ListBox as Log destination
	GetCore()->GetLog().SetListCtrl( &logListBoxCtrl );

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
	GetCore()->GetLog().SetListCtrl( NULL );
	SaveToRegsitry();
	// Stop using ListBox as log destination
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
	GetRegistry()->WriteProfileStringW( _T( "Paths" ), _T( "outputFolder" ), outputFolder );
}

DWORD WINAPI Launch( LPVOID lpParam )
{
	CNeuroLogDlg* dlg = ( CNeuroLogDlg* )lpParam;

	GetCore()->GetLog().Add( L" " );

	ChronoTimer timer;
	timer.Start();

	GetCore()->LoadSubnets();
	GetCore()->LoadLogs();
	GetCore()->AnalyzeSubnets();
	GetCore()->ClearData();

	timer.Stop();
	if ( timer.GetElapsedMillisecondsRaw().count() > 0 && GetCore()->totalHitsSession > 0 )
	{
		GetCore()->GetLog().Add( L"Total speed: " + std::to_wstring( GetCore()->totalHitsSession / timer.GetElapsedMillisecondsRaw().count() ) + L" hits/ms" );
	}
	dlg->UnlockDlg( TRUE );

	return 0;
}

void CNeuroLogDlg::OnBnClickedButtonStart()
{
	SaveToRegsitry();

	if ( subnetsDbFolder.IsEmpty() )
	{
		AfxMessageBox(_T("Where are subnet files?"));
		return;
	}
	GetCore()->subnetsFolder = subnetsDbFolder;


	if ( logsFolder.IsEmpty() )
	{
		AfxMessageBox(_T("Where are log files?"));
		return;
	}
	GetCore()->logsFolder = logsFolder;


	if ( logsMask.IsEmpty() )
	{
		AfxMessageBox( _T( "Where is logs mask?" ) );
		return;
	}
	GetCore()->logsMask = logsMask;


	if ( outputFolder.IsEmpty() )
	{
		AfxMessageBox(_T("We need an output folder."));
		return;
	}
	GetCore()->outputFolder = outputFolder;


	GetCore()->hitsLimit = hitsLimit;
	GetCore()->sizeLimit = sizeLimit;

	CWaitCursor wait;
	GetDlgItem( IDC_BUTTON_START )->EnableWindow( FALSE );

	HANDLE hThread = CreateThread( NULL, 0, Launch, this, 0, NULL );

	/*
	GetCore()->LoadSubnets();
	GetCore()->LoadLogs();
	GetCore()->AnalyzeSubnets();

	GetDlgItem( IDC_BUTTON_START )->EnableWindow( TRUE );

	GetCore()->ClearData();
	*/
	wait.Restore();
}

BOOL CNeuroLogDlg::PickTheFolder( CWnd* mfcWnd, CString* pcsFolder )
{
	BOOL boolResult = FALSE;
	LPMALLOC pMalloc;
	BROWSEINFO bi;
	LPITEMIDLIST pidl;

	::ZeroMemory( &bi, sizeof( bi ) );
	TCHAR tcSelectedFolder[ MAX_PATH ];
	memset( tcSelectedFolder, 0, sizeof( tcSelectedFolder ) );

	// Gets the Shell's default allocator
	if ( ::SHGetMalloc( &pMalloc ) == NOERROR )
	{
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		if ( mfcWnd != NULL )
		{
			bi.hwndOwner = mfcWnd->m_hWnd;
		}
		bi.pidlRoot = NULL;
		bi.pszDisplayName = tcSelectedFolder;
		bi.lpszTitle = _T("Select folder please:");
		bi.ulFlags = 0x40 | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_EDITBOX;
		bi.lpfn = NULL;
		bi.lParam = NULL;
		// This next call issues the dialog box.
		if (( pidl = ::SHBrowseForFolder( &bi )) != NULL )
		{
			if ( ::SHGetPathFromIDList( pidl, tcSelectedFolder ) )
			{
				// At this point pszBuffer contains the selected path
				pcsFolder->Empty();
				*pcsFolder = tcSelectedFolder;
				boolResult = TRUE;
			} // if
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free( pidl );
		} // if
		// Release the shell's allocator.
		pMalloc->Release();
	} // if

	return boolResult;
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

void CNeuroLogDlg::OnClickedButtonOutput()
{
	UpdateData( TRUE );
	PickTheFolder( this, &outputFolder );
	UpdateData( FALSE );
}

void CNeuroLogDlg::UnlockDlg( BOOL unlockValue )
{
	GetDlgItem( IDC_BUTTON_START )->EnableWindow( unlockValue );
}


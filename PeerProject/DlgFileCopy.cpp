//
// DlgFileCopy.cpp
//
// This file is part of PeerProject (peerproject.org) � 2008-2011
// Portions copyright Shareaza Development Team, 2002-2007.
//
// PeerProject is free software; you can redistribute it and/or
// modify it under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// either version 3 of the License, or later version at your option.
//
// PeerProject is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 (AGPLv3) for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#include "StdAfx.h"
#include "Settings.h"
#include "PeerProject.h"
#include "DlgFileCopy.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "CtrlSharedFolder.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif	// Filename

BEGIN_MESSAGE_MAP(CFileCopyDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CFileCopyDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg dialog

CFileCopyDlg::CFileCopyDlg(CWnd* pParent, BOOL bMove)
	: CSkinDialog(CFileCopyDlg::IDD, pParent)
	, m_bMove		( bMove )
	, m_nFileProg	( 0 )
	, m_nCookie 	( 0 )
	, m_bCancel		( FALSE )
{
}

void CFileCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileCopyDlg)
	DDX_Control(pDX, IDC_MESSAGE_MOVE, m_wndMove);
	DDX_Control(pDX, IDC_MESSAGE_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndFileName);
	DDX_Control(pDX, IDC_PROGRESS_FILE, m_wndFileProg);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PLACEHOLDER, m_wndPlaceholder);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg message handlers

void CFileCopyDlg::AddFile(CLibraryFile* pFile)
{
	m_pFiles.AddTail( pFile->m_nIndex );
}

BOOL CFileCopyDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CFileCopyDlg"), IDR_LIBRARYFRAME );
	SelectCaption( this, m_bMove ? 1 : 0 );

	CString strCaption, strBase;

	CWnd* pMessage = m_bMove ? &m_wndMove : &m_wndCopy;
	pMessage->GetWindowText( strBase );
	strCaption.Format( strBase, m_pFiles.GetCount() );
	pMessage->SetWindowText( strCaption );
	pMessage->ShowWindow( SW_SHOW );

	if ( m_pFiles.GetCount() == 1 )
		GetDlgItem( IDC_PROGRESS )->EnableWindow( FALSE );

	CRect rc;
	m_wndPlaceholder.GetWindowRect( &rc );
	ScreenToClient( &rc );
	if ( ! m_wndTree.Create( WS_VISIBLE|WS_TABSTOP|WS_BORDER, rc, this, IDC_FOLDERS ) ) return -1;
	m_wndTree.SetMultiSelect( FALSE );

	{
		CQuickLock oLock( Library.m_pSection );

		m_nCookie = Library.GetCookie();
		m_wndTree.Update();

		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sTarget ) )
			m_wndTree.SelectFolder( pFolder );
	}

	if ( Settings.General.LanguageRTL )
	{
		m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndFileProg.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}
	m_wndFileProg.SetRange( 0, 400 );

	PostMessage( WM_TIMER, 1 );
	SetTimer( 1, 500, NULL );

	return TRUE;
}

void CFileCopyDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	if ( IsThreadCompleted() )
	{
		StopOperation();
		PostMessage( WM_COMMAND, IDCANCEL );
		return;
	}

	if ( ! m_wndTree.IsWindowEnabled() ) return;

	if ( m_nCookie != Library.GetCookie() )
	{
		CSingleLock pLock( &Library.m_pSection );

		if ( pLock.Lock( 500 ) )
		{
			m_nCookie = Library.GetCookie();
			m_wndTree.Update();
		}
	}

	m_wndOK.EnableWindow( m_wndTree.GetSelectedFolderIterator() != NULL );
}

BOOL CFileCopyDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNotify = (NMHDR*)lParam;

	if ( pNotify->code == NM_DBLCLK )
	{
		PostMessage( WM_COMMAND, IDOK );
		return TRUE;
	}

	return CSkinDialog::OnNotify(wParam, lParam, pResult);
}

void CFileCopyDlg::OnOK()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION pos = m_wndTree.GetSelectedFolderIterator();
	if ( pos == NULL ) return;

	CLibraryFolder* pFolder = m_wndTree.GetNextSelectedFolder( pos );

	m_sTarget = pFolder->m_sPath;

	pLock.Unlock();

	StartOperation();
}

void CFileCopyDlg::OnCancel()
{
	if ( IsThreadAlive() )
	{
		StopOperation();
		m_wndFileName.SetWindowText( _T("Operation cancelled") );
		return;
	}

	StopOperation();

	CSkinDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation control

void CFileCopyDlg::StartOperation()
{
	if ( IsThreadAlive() ) return;

	m_wndTree.EnableWindow( FALSE );
	m_wndOK.EnableWindow( FALSE );

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndProgress.SetPos( 0 );

	m_bCancel = FALSE;
	BeginThread( "DlgFileCopy" );
}

void CFileCopyDlg::StopOperation()
{
	if ( ! IsThreadAlive() ) return;

	CWaitCursor pCursor;

	CloseThread();

	//m_wndCancel.SetWindowText( _T("&Close") );
	CString sText;
	LoadString ( sText, IDS_GENERAL_CLOSE );
	m_wndCancel.SetWindowText( sText );
	m_wndProgress.EnableWindow( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation thread

void CFileCopyDlg::OnRun()
{
	while ( IsThreadEnabled() )
	{
		CString strName, strPath;
		CSchemaPtr pSchema = NULL;
		CXMLElement* pMetadata = NULL;
		BOOL bMetadataAuto = FALSE;
		int nRating = 0;
		CString sComments;
		CString sShareTags;
		CLibraryFile* pFile;
		{
			CQuickLock oLock( Library.m_pSection );

			if ( m_pFiles.IsEmpty() ) break;

			DWORD nIndex = m_pFiles.RemoveHead();

			pFile = Library.LookupFile( nIndex );

			if ( pFile != NULL && pFile->IsAvailable() )
			{
				strName 	= pFile->m_sName;
				strPath 	= pFile->GetFolder();
				pSchema 	= pFile->m_pSchema;
				pMetadata	= pFile->m_pMetadata ? pFile->m_pMetadata->Clone() : NULL;
				bMetadataAuto = pFile->m_bMetadataAuto;
				nRating 	= pFile->m_nRating;
				sComments	= pFile->m_sComments;
				sShareTags	= pFile->m_sShareTags;
			}
		}

		if ( pFile == NULL || ! pFile->IsAvailable() ) break;

		m_wndProgress.OffsetPos( 1 );

		m_wndFileName.SetWindowText( strName );

		if ( ProcessFile( strName, strPath ) )
		{
			CQuickLock oLock( Library.m_pSection );

			CLibraryFolder* pTargetFolder = LibraryFolders.GetFolder( m_sTarget );
			if ( pTargetFolder )
			{
				BOOL bNew;
				CLibraryFile* pTargetFile = pTargetFolder->AddFile( strName, bNew );
				if ( pSchema )
				{
					pTargetFile->m_pSchema = pSchema;
					pSchema = NULL;
				}
				pTargetFile->m_bMetadataAuto = bMetadataAuto;
				pTargetFile->m_nRating = nRating;
				pTargetFile->m_sComments = sComments;
				pTargetFile->m_sShareTags = sShareTags;
				if ( pMetadata )
				{
					if ( pTargetFile->m_pMetadata )
					{
						pMetadata->Merge( pTargetFile->m_pMetadata );
						delete pTargetFile->m_pMetadata;
					}
					pTargetFile->m_pMetadata = pMetadata;
					pMetadata = NULL;
					pTargetFile->ModifyMetadata();
				}

				if ( bNew )
					Library.AddFile( pTargetFile );
			}
		}

		delete pMetadata;

	// Alternate code to check if file is hashing first:

	//	CString sCurrent, sFile;
	//	int nRemaining;
	//	LibraryBuilder.UpdateStatus( &sCurrent, &nRemaining );
	//	sFile = strPath + _T("\\") + strName;
	//	if ( sFile == sCurrent )
	//	{
	//		LoadString ( sFile, IDS_LIBRARY_BITZI_HASHED );
	//		sCurrent.Format( sFile, strName );
	//		theApp.Message( MSG_NOTICE, sCurrent );
	//		LoadString ( sCurrent, IDS_STATUS_FILEERROR );
	//		m_wndFileName.SetWindowText( sCurrent );
	//	}
	//	else
	//	{
	//		m_wndFileName.SetWindowText( strName );
	//		ProcessFile( strName, strPath, bMetaData );
	//	}
	}
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg file processing

bool CFileCopyDlg::ProcessFile(const CString& strName, const CString& strPath)
{
	if ( ! strPath.CompareNoCase( m_sTarget ) )
		return false;

	const CString sSource = strPath + _T("\\") + strName;
	const CString sTarget = m_sTarget + _T("\\") + strName;

	// Check if we can move the file first
	if ( m_bMove )
	{
		HANDLE hFile = CreateFile( sSource, GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		VERIFY_FILE_ACCESS( hFile, sSource )
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_LIBRARY_MOVE_FAIL ), strName );

			switch ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) )
			{
			case IDYES:
				m_bMove = FALSE;
				break;
			case IDNO:
				break;
			default:
				return false;
			}
		}
		CloseHandle( hFile );
	}

	// Copy/Move the file
	if ( m_bMove )
		return ProcessMove( sSource, sTarget );
	else
		return ProcessCopy( sSource, sTarget );
}

bool CFileCopyDlg::CheckTarget(const CString& strTarget)
{
	if ( GetFileAttributes( strTarget ) == 0xFFFFFFFF )
		return true;

	CString strFormat, strMessage;

	LoadString( strFormat, IDS_LIBRARY_TARGET_EXISTS );
	strMessage.Format( strFormat, strTarget );

	switch ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 ) )
	{
	case IDYES:
		break;
	case IDCANCEL:
		Exit();
	case IDNO:
	default:
		return false;
	}

	if ( DeleteFileEx( strTarget, TRUE, FALSE, FALSE ) )
		return true;

	CString strError = GetErrorString();

	LoadString( strFormat, IDS_LIBRARY_DELETE_FAIL );
	strMessage.Format( strFormat, strTarget );
	strMessage += _T("\r\n\r\n") + strError;

	AfxMessageBox( strMessage, MB_ICONEXCLAMATION );

	return false;
}

bool CFileCopyDlg::ProcessMove(const CString& strSource, const CString& strTarget)
{
	if ( ! CheckTarget( strTarget ) )
		return false;

	// Close the file handle
	theApp.OnRename( strSource );

	// Try moving the file
	if ( MoveFile( strSource, strTarget ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( strSource, strTarget );
		return true;
	}

	// Try a copy/delete. (Will usually make a duplicate of the file)
	if ( ProcessCopy( strSource, strTarget ) )
	{
		// Success. Tell the file to use its new name
		theApp.OnRename( strSource, strTarget );
		return DeleteFileEx( strSource, TRUE, FALSE, FALSE ) != 0;
	}

	// Failure. Continue using its old name
	theApp.OnRename( strSource, strSource );

	return false;
}

bool CFileCopyDlg::ProcessCopy(const CString& strSource, const CString& strTarget)
{
	if ( ! CheckTarget( strTarget ) )
		return false;

	m_wndFileProg.SetPos( 0 );
	m_nFileProg = 0;

	bool bResult = CopyFileEx( strSource, strTarget, CopyCallback, this,
		&m_bCancel, COPY_FILE_FAIL_IF_EXISTS ) != 0;

	if ( ! bResult && ! IsThreadAlive() )
		DeleteFileEx( strTarget, TRUE, FALSE, FALSE );

	return bResult;
}

DWORD WINAPI CFileCopyDlg::CopyCallback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/, LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/, DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/, LPVOID lpData)
{
	CFileCopyDlg* pDlg = (CFileCopyDlg*)lpData;

	if ( ! pDlg->IsThreadAlive() ) return 1;

	if ( TotalFileSize.LowPart )
	{
		double nProgress = ( (double)TotalBytesTransferred.LowPart / (double)TotalFileSize.LowPart );
		int iProgress = (int)( nProgress * 400 );

		if ( iProgress != pDlg->m_nFileProg )
			pDlg->m_wndFileProg.SetPos( pDlg->m_nFileProg = iProgress );
	}

	return 0;
}

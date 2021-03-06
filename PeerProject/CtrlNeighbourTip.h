//
// CtrlNeighbourTip.h
//
// This file is part of PeerProject (peerproject.org) � 2008-2012
// Portions copyright Shareaza Development Team, 2002-2007.
//
// PeerProject is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// PeerProject is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

#include "CtrlCoolTip.h"

class CGraphItem;


class CNeighbourTipCtrl : public CCoolTipCtrl
{
	DECLARE_DYNAMIC(CNeighbourTipCtrl)

public:
	CNeighbourTipCtrl();
	virtual ~CNeighbourTipCtrl();

public:
	void Show(DWORD_PTR pContext, HWND hAltWnd = NULL)
	{
		bool bChanged = ( pContext != m_nNeighbour );
		m_nNeighbour = pContext;
		m_hAltWnd = hAltWnd;
		ShowImpl( bChanged );
	}

protected:
	DWORD_PTR	m_nNeighbour;
	CLineGraph*	m_pGraph;
	CGraphItem*	m_pItemIn;
	CGraphItem*	m_pItemOut;
//	CImageList	m_pProtocols;

	virtual BOOL OnPrepare();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnCalcSize(CDC* pDC);
	virtual void OnPaint(CDC* pDC);

// Overrides
//public:
	//{{AFX_VIRTUAL(CNeighbourTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CNeighbourTipCtrl)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

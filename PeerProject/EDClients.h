//
// EDClients.h
//
// This file is part of PeerProject (peerproject.org) � 2008-2012
// Portions copyright Shareaza Development Team, 2002-2008.
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

#include "EDClient.h"

class CConnection;
class CEDPacket;


class CEDClients
{
// Construction
public:
	CEDClients();
	virtual ~CEDClients();

private:
	CEDClients(const CEDClients&);				// Declaration only
	CEDClients& operator=(const CEDClients&);	// Declaration only

// Attributes
public:
	mutable CMutex	m_pSection;		// EDClients Guard
private:
	CEDClient*		m_pFirst;
	CEDClient*		m_pLast;
	int				m_nCount;
	DWORD			m_tLastRun;
	DWORD			m_tLastMaxClients;
	DWORD			m_tLastServerStats;
	in_addr			m_pLastServer;
	DWORD			m_nLastServerKey;
	BOOL			m_bAllServersDone;

// Operations
public:
	void			Add(CEDClient* pClient);
	void			Remove(CEDClient* pClient);
	void			Clear();
	int				GetCount() const;
	bool			PushTo(DWORD nClientID, WORD nClientPort);
	CEDClient*		GetByIP(const IN_ADDR* pAddress) const;
	CEDClient*		Connect(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID);
					// Connect to new or known eD2K-client (nClientPort and nServerPort must be in host byte order)
	BOOL			Merge(CEDClient* pClient);
	void			OnRun();
	BOOL			OnAccept(CConnection* pConnection);
	BOOL			OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	bool			IsFull(const CEDClient* pCheckThis = NULL);
	BOOL			IsOverloaded() const;
	BOOL			IsMyDownload(const CDownloadTransferED2K* pDownload) const;

protected:
	CEDClient*		GetByID(DWORD nClientID, IN_ADDR* pServer, const Hashes::Guid& oGUID) const;
	CEDClient*		GetByGUID(const Hashes::Guid& oGUID) const;
	void			OnServerStatus(const SOCKADDR_IN* pHost, CEDPacket* pPacket);
	void			RequestServerStatus(IN_ADDR* pHost, WORD nPort);
	void			RunGlobalStatsRequests(DWORD tNow);
};

extern CEDClients EDClients;

//
// DCPacket.h
//
// This file is part of PeerProject (peerproject.org) � 2010
// Portions copyright Shareaza Development Team, 2010.
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

#pragma once

#include "Packet.h"

class CDCPacket : public CPacket
{
protected:
	CDCPacket();
	virtual ~CDCPacket();

public:
	virtual void Reset();
	virtual void ToBuffer(CBuffer* pBuffer, bool bTCP = true) const;

#ifdef _DEBUG
	virtual void Debug(LPCTSTR pszReason) const;	// Writes debug info about packet into the PeerProject.log file
#endif

// Packet Pool
protected:
	class CDCPacketPool : public CPacketPool
	{
	public:
		virtual ~CDCPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};

	static CDCPacketPool POOL;

// Allocation
public:
	inline static CDCPacket* New(const BYTE* pBuffer = NULL, DWORD nLength = 0)
	{
		if ( CDCPacket* pPacket = (CDCPacket*)POOL.New() )
		{
			if ( pBuffer == NULL || pPacket->Write( pBuffer, nLength ) )
				return pPacket;
			pPacket->Release();
		}
		return NULL;
	}

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

protected:
	BOOL OnCommonHit(const SOCKADDR_IN* pHost);

	friend class CDCPacket::CDCPacketPool;

private:
	CDCPacket(const CDCPacket&);
	CDCPacket& operator=(const CDCPacket&);
};

inline void CDCPacket::CDCPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CDCPacket);
	pPool	= new CDCPacket[ nSize ];
}

inline void CDCPacket::CDCPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CDCPacket*)pPacket;
}

#define DC_PROTOCOL_MIN_LEN	3
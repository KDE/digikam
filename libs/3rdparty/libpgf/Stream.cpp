/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2007-01-19 11:51:24 +0100 (Fr, 19 Jan 2007) $
 * $Revision: 268 $
 * 
 * This file Copyright (C) 2006 xeraina GmbH, Switzerland
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// Stream.cpp: implementation of the CStream class.
//
//////////////////////////////////////////////////////////////////////

#include "PGFtypes.h"
#include "Stream.h"

#ifdef WIN32
#include <malloc.h>
#endif

//////////////////////////////////////////////////////////////////////
// CPGFFileStream
//////////////////////////////////////////////////////////////////////
void CPGFFileStream::Write(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	OSError err;
	if ((err = FileWrite(m_hFile, count, buffPtr)) != NoError) ReturnWithError(err);
	
}

//////////////////////////////////////////////////////////////////////
void CPGFFileStream::Read(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	OSError err;
	if ((err = FileRead(m_hFile, count, buffPtr)) != NoError) ReturnWithError(err);
}

//////////////////////////////////////////////////////////////////////
void CPGFFileStream::SetPos(short posMode, INT64 posOff) THROW_ {
	ASSERT(IsValid());
	OSError err;
	if ((err = SetFPos(m_hFile, posMode, posOff)) != NoError) ReturnWithError(err);
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFFileStream::GetPos() const THROW_ {
	ASSERT(IsValid());
	OSError err;
	UINT64 pos;
	if ((err = GetFPos(m_hFile, &pos)) != NoError) ReturnWithError2(err, pos);
	return pos;
}


//////////////////////////////////////////////////////////////////////
// CPGFMemoryStream
//////////////////////////////////////////////////////////////////////
// allocate memory block of given size
CPGFMemoryStream::CPGFMemoryStream(size_t size) THROW_ : m_size(size), m_allocated(true) {
	m_buffer = m_pos = new UINT8[m_size];
	if (!m_buffer) ReturnWithError(InsufficientMemory);
}

CPGFMemoryStream::CPGFMemoryStream(UINT8 *pBuffer, size_t size) THROW_ : m_buffer(pBuffer), m_pos(pBuffer), m_size(size), m_allocated(false) {
	ASSERT(IsValid());
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::Write(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	const size_t deltaSize = 0x4000 + *count;
	
	if (m_pos + *count <= m_buffer + m_size) {
		memcpy(m_pos, buffPtr, *count);
		m_pos += *count;
	} else if (m_allocated) {
		// memory block is too small -> reallocate a deltaSize larger block
		size_t offset = m_pos - m_buffer;
		m_buffer = (UINT8 *)realloc(m_buffer, m_size + deltaSize);
		if (!m_buffer) ReturnWithError(InsufficientMemory);
		m_size += deltaSize;

		// reposition m_pos
		m_pos = m_buffer + offset;

		// write block
		memcpy(m_pos, buffPtr, *count);
		m_pos += *count;
	} else {
		ReturnWithError(InsufficientMemory);	
	}
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::Read(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	
	if (m_pos + *count <= m_buffer + m_size) {
		memcpy(buffPtr, m_pos, *count);
		m_pos += *count;
	} else {
		// end of memory block reached -> read only until end
		*count = (long)(m_buffer + m_size - m_pos);
		memcpy(buffPtr, m_pos, *count);
		m_pos += *count;
		ReturnWithError(EndOfMemory);
	}
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::SetPos(short posMode, INT64 posOff) THROW_ {
	ASSERT(IsValid());
	switch(posMode) {
	case FSFromStart:
		m_pos = m_buffer + posOff;
		break;
	case FSFromCurrent:
		m_pos += posOff;
		break;
	case FSFromEnd:
		m_pos = m_buffer + m_size + posOff;
		break;
	default:
		ASSERT(false);
	}
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFMemoryStream::GetPos() const THROW_ {
	ASSERT(IsValid());
	return m_pos - m_buffer;
}


//////////////////////////////////////////////////////////////////////
// CPGFMemFileStream
//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::Write(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
#ifdef _MFC_VER
	m_memFile->Write(buffPtr, *count);
#endif
}

//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::Read(long *count, void *buffPtr) THROW_ {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
#ifdef _MFC_VER
	m_memFile->Read(buffPtr, *count);
#endif
}

//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::SetPos(short posMode, INT64 posOff) THROW_ {
	ASSERT(IsValid());
#ifdef _MFC_VER
	m_memFile->Seek(posOff, posMode); 
#endif
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFMemFileStream::GetPos() const THROW_ {
#ifdef _MFC_VER
	return (ULONG)m_memFile->GetPosition();
#else
	return 0;
#endif
}

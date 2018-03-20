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

//////////////////////////////////////////////////////////////////////
/// @file PGFstream.cpp
/// @brief PGF stream class implementation
/// @author C. Stamm

#include "PGFstream.h"

#ifdef WIN32
#include <malloc.h>
#endif

//////////////////////////////////////////////////////////////////////
// CPGFFileStream
//////////////////////////////////////////////////////////////////////
void CPGFFileStream::Write(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	OSError err;
	if ((err = FileWrite(m_hFile, count, buffPtr)) != NoError) ReturnWithError(err);

}

//////////////////////////////////////////////////////////////////////
void CPGFFileStream::Read(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	OSError err;
	if ((err = FileRead(m_hFile, count, buffPtr)) != NoError) ReturnWithError(err);
}

//////////////////////////////////////////////////////////////////////
void CPGFFileStream::SetPos(short posMode, INT64 posOff) {
	ASSERT(IsValid());
	OSError err;
	if ((err = SetFPos(m_hFile, posMode, posOff)) != NoError) ReturnWithError(err);
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFFileStream::GetPos() const {
	ASSERT(IsValid());
	OSError err;
	UINT64 pos = 0;
	if ((err = GetFPos(m_hFile, &pos)) != NoError) ReturnWithError2(err, pos);
	return pos;
}


//////////////////////////////////////////////////////////////////////
// CPGFMemoryStream
//////////////////////////////////////////////////////////////////////
/// Allocate memory block of given size
/// @param size Memory size
CPGFMemoryStream::CPGFMemoryStream(size_t size)
: m_size(size)
, m_allocated(true) {
	m_buffer = m_pos = m_eos = new(std::nothrow) UINT8[m_size];
	if (!m_buffer) ReturnWithError(InsufficientMemory);
}

//////////////////////////////////////////////////////////////////////
/// Use already allocated memory of given size
/// @param pBuffer Memory location
/// @param size Memory size
CPGFMemoryStream::CPGFMemoryStream(UINT8 *pBuffer, size_t size)
: m_buffer(pBuffer)
, m_pos(pBuffer)
, m_eos(pBuffer + size)
, m_size(size)
, m_allocated(false) {
	ASSERT(IsValid());
}

//////////////////////////////////////////////////////////////////////
/// Use already allocated memory of given size
/// @param pBuffer Memory location
/// @param size Memory size
void CPGFMemoryStream::Reinitialize(UINT8 *pBuffer, size_t size) {
	if (!m_allocated) {
		m_buffer = m_pos = pBuffer;
		m_size = size;
		m_eos = m_buffer + size;
	}
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::Write(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	const size_t deltaSize = 0x4000 + *count;

	if (m_pos + *count <= m_buffer + m_size) {
		memcpy(m_pos, buffPtr, *count);
		m_pos += *count;
		if (m_pos > m_eos) m_eos = m_pos;
	} else if (m_allocated) {
		// memory block is too small -> reallocate a deltaSize larger block
		size_t offset = m_pos - m_buffer;
		UINT8 *buf_tmp = (UINT8 *)realloc(m_buffer, m_size + deltaSize);
		if (!buf_tmp) {
			delete[] m_buffer;
			m_buffer = 0;
			ReturnWithError(InsufficientMemory);
		} else {
			m_buffer = buf_tmp;
		}
		m_size += deltaSize;

		// reposition m_pos
		m_pos = m_buffer + offset;

		// write block
		memcpy(m_pos, buffPtr, *count);
		m_pos += *count;
		if (m_pos > m_eos) m_eos = m_pos;
	} else {
		ReturnWithError(InsufficientMemory);
	}
	ASSERT(m_pos <= m_eos);
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::Read(int *count, void *buffPtr) {
	ASSERT(IsValid());
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(m_buffer + m_size >= m_eos);
	ASSERT(m_pos <= m_eos);

	if (m_pos + *count <= m_eos) {
		memcpy(buffPtr, m_pos, *count);
		m_pos += *count;
	} else {
		// end of memory block reached -> read only until end
		*count = (int)__max(0, m_eos - m_pos);
		memcpy(buffPtr, m_pos, *count);
		m_pos += *count;
	}
	ASSERT(m_pos <= m_eos);
}

//////////////////////////////////////////////////////////////////////
void CPGFMemoryStream::SetPos(short posMode, INT64 posOff) {
	ASSERT(IsValid());
	switch(posMode) {
	case FSFromStart:
		m_pos = m_buffer + posOff;
		break;
	case FSFromCurrent:
		m_pos += posOff;
		break;
	case FSFromEnd:
		m_pos = m_eos + posOff;
		break;
	default:
		ASSERT(false);
	}
	if (m_pos > m_eos)
		ReturnWithError(InvalidStreamPos);
}


//////////////////////////////////////////////////////////////////////
// CPGFMemFileStream
#ifdef _MFC_VER
//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::Write(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	m_memFile->Write(buffPtr, *count);
}

//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::Read(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());
	m_memFile->Read(buffPtr, *count);
}

//////////////////////////////////////////////////////////////////////
void CPGFMemFileStream::SetPos(short posMode, INT64 posOff) {
	ASSERT(IsValid());
	m_memFile->Seek(posOff, posMode);
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFMemFileStream::GetPos() const {
	return (UINT64)m_memFile->GetPosition();
}
#endif // _MFC_VER

//////////////////////////////////////////////////////////////////////
// CPGFIStream
#if defined(WIN32) || defined(WINCE)
//////////////////////////////////////////////////////////////////////
void CPGFIStream::Write(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());

	HRESULT hr = m_stream->Write(buffPtr, *count, (ULONG *)count);
	if (FAILED(hr)) {
		ReturnWithError(hr);
	}
}

//////////////////////////////////////////////////////////////////////
void CPGFIStream::Read(int *count, void *buffPtr) {
	ASSERT(count);
	ASSERT(buffPtr);
	ASSERT(IsValid());

	HRESULT hr = m_stream->Read(buffPtr, *count, (ULONG *)count);
	if (FAILED(hr)) {
		ReturnWithError(hr);
	}
}

//////////////////////////////////////////////////////////////////////
void CPGFIStream::SetPos(short posMode, INT64 posOff) {
	ASSERT(IsValid());

	LARGE_INTEGER li;
	li.QuadPart = posOff;

	HRESULT hr = m_stream->Seek(li, posMode, nullptr);
	if (FAILED(hr)) {
		ReturnWithError(hr);
	}
}

//////////////////////////////////////////////////////////////////////
UINT64 CPGFIStream::GetPos() const {
	ASSERT(IsValid());

	LARGE_INTEGER n;
	ULARGE_INTEGER pos;
	n.QuadPart = 0;

	HRESULT hr = m_stream->Seek(n, FSFromCurrent, &pos);
	if (SUCCEEDED(hr)) {
		return pos.QuadPart;
	} else {
		ReturnWithError2(hr, pos.QuadPart);
	}
}
#endif // WIN32 || WINCE

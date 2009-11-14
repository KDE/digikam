/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2007-06-11 10:56:17 +0200 (Mo, 11 Jun 2007) $
 * $Revision: 299 $
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

#ifndef PGF_STREAM_H
#define PGF_STREAM_H

#include "PGFtypes.h"

/////////////////////////////////////////////////////////////////////
/// Abstract stream base class.
/// @author C. Stamm
class CPGFStream {
public:
	//////////////////////////////////////////////////////////////////////
	/// Standard constructor.
	CPGFStream() {}

	//////////////////////////////////////////////////////////////////////
	/// Standard destructor.
	virtual ~CPGFStream() {}

	//////////////////////////////////////////////////////////////////////
	/// Write some bytes out of a buffer into this stream.
	/// @param count A pointer to a value containing the number of bytes should be written. After this call it contains the number of written bytes.
	/// @param buffer A memory buffer
	virtual void Write(int *count, void *buffer)=0; 

	//////////////////////////////////////////////////////////////////////
	/// Read some bytes from this stream and stores them into a buffer.
	/// @param count A pointer to a value containing the number of bytes should be read. After this call it contains the number of read bytes.
	/// @param buffer A memory buffer
	virtual void Read(int *count, void *buffer)=0; 

	//////////////////////////////////////////////////////////////////////
	/// Set stream position either absolute or relative.
	/// @param posMode A position mode (FSFromStart, FSFromCurrent, FSFromEnd)
	/// @param posOff A new stream position (absolute positioning) or a position offset (relative positioning)
	virtual void SetPos(short posMode, INT64 posOff)=0;

	//////////////////////////////////////////////////////////////////////
	/// Get current stream position.
	/// @return Current stream position
	virtual UINT64  GetPos() const=0;

	//////////////////////////////////////////////////////////////////////
	/// Check stream validity.
	/// @return True if stream and current position is valid
	virtual bool	IsValid() const=0;
};

/////////////////////////////////////////////////////////////////////
/// A PGF stream subclass for external storage files.
/// @author C. Stamm
class CPGFFileStream : public CPGFStream {
protected:
	HANDLE m_hFile;

public:
	CPGFFileStream() : m_hFile(0) {}
	/// Constructor
	/// @param hFile File handle
	CPGFFileStream(HANDLE hFile) : m_hFile(hFile) {}
	/// @return File handle
	HANDLE GetHandle() { return m_hFile; }

	virtual ~CPGFFileStream() { m_hFile = 0; }
	virtual void Write(int *count, void *buffer) THROW_; // throws IOException 
	virtual void Read(int *count, void *buffer) THROW_; // throws IOException 
	virtual void SetPos(short posMode, INT64 posOff) THROW_; // throws IOException
	virtual UINT64 GetPos() const THROW_; // throws IOException
	virtual bool   IsValid() const	{ return m_hFile != 0; }
};

/////////////////////////////////////////////////////////////////////
/// A PGF stream subclass for internal memory.
/// @author C. Stamm
class CPGFMemoryStream : public CPGFStream {
protected:
	UINT8 *m_buffer, *m_pos;
	size_t m_size;
	bool   m_allocated;

public:
	/// Constructor
	/// @param size Size of new allocated memory buffer
	CPGFMemoryStream(size_t size) THROW_;
	/// Constructor. Use already allocated memory of given size
	/// @param pBuffer Memory location
	/// @param size Memory size
	CPGFMemoryStream(UINT8 *pBuffer, size_t size) THROW_;
	virtual ~CPGFMemoryStream() { 
		m_pos = 0; 
		if (m_allocated) {
			// the memory buffer has been allocated inside of CPGFMemoryStream constructor
			delete[] m_buffer; m_buffer = 0;
		}
	}

	virtual void Write(int *count, void *buffer) THROW_; // throws IOException 
	virtual void Read(int *count, void *buffer) THROW_; // throws IOException 
	virtual void SetPos(short posMode, INT64 posOff) THROW_; // throws IOException
	virtual UINT64 GetPos() const THROW_; // throws IOException
	virtual bool   IsValid() const	{ return m_buffer != 0; }

	/// @return Memory size
	size_t GetSize() const			{ return m_size; }
	/// @return Memory buffer 
	const UINT8* GetBuffer() const	{ return m_buffer; }
	/// @return Memory buffer
	UINT8* GetBuffer()				{ return m_buffer; }
};

/////////////////////////////////////////////////////////////////////
/// A PGF stream subclass for internal memory files. Usable only with MFC.
/// @author C. Stamm
#ifdef _MFC_VER
class CPGFMemFileStream : public CPGFStream {
protected:
	CMemFile *m_memFile;
public:
	CPGFMemFileStream(CMemFile *memFile) : m_memFile(memFile) {}
	virtual bool	IsValid() const	{ return m_memFile != NULL; }
	virtual ~CPGFMemFileStream() {}
	virtual void Write(int *count, void *buffer) THROW_; // throws IOException 
	virtual void Read(int *count, void *buffer) THROW_; // throws IOException 
	virtual void SetPos(short posMode, INT64 posOff) THROW_; // throws IOException
	virtual UINT64 GetPos() const THROW_; // throws IOException
};
#endif

/////////////////////////////////////////////////////////////////////
/// A PGF stream subclass for IStream. Usable only with COM.
/// @author C. Stamm
#if defined(WIN32) || defined(WINCE)
class CPGFIStream : public CPGFStream {
protected:
	IStream *m_stream;
public:
	CPGFIStream(IStream *stream) : m_stream(stream) {}
	virtual bool IsValid() const	{ return m_stream != 0; }
	virtual ~CPGFIStream() {}
	virtual void Write(int *count, void *buffer) THROW_; // throws IOException 
	virtual void Read(int *count, void *buffer) THROW_; // throws IOException 
	virtual void SetPos(short posMode, INT64 posOff) THROW_; // throws IOException
	virtual UINT64 GetPos() const THROW_; // throws IOException
};
#endif

#endif // PGF_STREAM_H

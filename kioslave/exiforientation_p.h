/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-10-24
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef EXIFORIENTATION_P_H
#define EXIFORIENTATION_P_H

#include <qfile.h>
#include <qdatastream.h>

#include <kdebug.h>
#include <libkexif/kexifdata.h>

/* TODO: this should be move into libkexif for the next version */

static KExifData::ImageOrientation getExifOrientation(const QString& path)
{
    //kdDebug() << "Reading file: " << path << endl;
    
    // position from start of exifsection
    unsigned int position = 0; 

    // stores single bytes read
    Q_UINT8 byte;

    QFile file( path );
    if (!file.open( IO_ReadOnly ))
    {
        //kdWarning() << "Failed to open file " << endl;
        return KExifData::UNSPECIFIED;
    }

    QDataStream stream( &file );

    // JPEG data is little endian
    stream.setByteOrder(QDataStream::LittleEndian);
 
    Q_UINT16 header;
    stream >> header;

    // check for valid JPEG header
    if(header != 0xd8ff)
    {
        //kdDebug() << "Not JPEG file." << endl;
       file.close();
       return KExifData::UNSPECIFIED;
    }

    // skip until EXIF marker is found
    while(!stream.atEnd())
    {
      while(byte != 0xff) {
        stream >> byte;
      }
      stream >> byte;

      // consume 0xff's used for padding
      while(byte == 0xff && !stream.atEnd())
        stream >> byte;

      // stop when we reach APP0 marker or start of image data
      if(byte == 0xe1 || byte == 0xc0)  // TODO: check more markers!
        break;
    }

    if(byte != 0xe1) {
        //kdDebug() << "No EXIF information found." << endl;
        file.close();
        return KExifData::UNSPECIFIED;
    }
    
    // get length of EXIF section
    Q_UINT16 sectionLen;
    stream >> sectionLen;

    // check for 'Exif' header
    Q_UINT8 exifHead[6];
    for( int i = 0; i < 6 ; i++ )
        stream >> exifHead[i];

    if( exifHead[0] != 0x45 || exifHead[1] != 0x78 || 
        exifHead[2] != 0x69 || exifHead[3] != 0x66 ||
        exifHead[4] != 0x00 || exifHead[5] != 0x00 ) {
        //kdDebug() << "No valid EXIF header found." << endl;
      file.close();
      return KExifData::UNSPECIFIED;
    }

    // get byte order of exif data
    Q_UINT16 byteOrder;
    stream >> byteOrder;

    if(byteOrder != 0x4949 && byteOrder != 0x4D4D)
    {
        //kdDebug() << "EXIF byte order could not be determined." << endl;
        file.close();
        return KExifData::UNSPECIFIED;
    } 

    // switch to Motorola byte order
    if(byteOrder == 0x4D4D)
    {
        stream.setByteOrder(QDataStream::BigEndian);
    } 

    // Check tag mark
    Q_UINT16 tagMark;
    stream >> tagMark;

    if( tagMark != 0x002A ) {
        //kdDebug() << "could not read EXIF tag mark." << endl;
      file.close();
      return KExifData::UNSPECIFIED;
    }

    // get offset of first IFD
    Q_UINT32 ifdOffset;
    stream >> ifdOffset;

    if( (Q_UINT16)ifdOffset > sectionLen - 2 || (Q_UINT16)ifdOffset < 2)
    {
        //kdDebug() << "Invalid IFD offset in EXIF data." << endl;
        file.close();
        return KExifData::UNSPECIFIED;
    }

    // We now read 8 bytes after start of Exif data
    position = 8;

    // seek forward to first IFD
    for(int i = 0 ; i < (Q_UINT16)ifdOffset - 8 ; i++ )
    {
        stream >> byte;
        position++;
    }

    
    QMemArray<unsigned char> buf(sectionLen);

    Q_UINT16 numberOfTags;
    stream >> numberOfTags;
    position += 2;
    //kdDebug() << "Number of EXIF tags in IFD0 section:" << numberOfTags << endl;
       
    int currentPosition = position;


    Q_UINT8 dummy;
    Q_UINT8 orientation = KExifData::UNSPECIFIED;
    
    for(int i=0 ; i < (sectionLen - currentPosition) ; i++)
    {
        stream >> byte;
        position++;
        buf[i] = byte;

        // search for Orientation tag
        // this code is not perfect, but the probability
        // that the sequence below is not
        // the Orientation tag is very small.
        if( byteOrder == 0x4D4D ) {
            if(i > 8  && buf[i]==0x01 && buf[i-1]==0x00
               && buf[i-2]==0x00 && buf[i-3]==0x00 
               && buf[i-4]==0x03 && buf[i-5]==0x00
               && buf[i-6]==0x12 && buf[i-7]==0x01)
            {
                stream >> dummy;
                stream >> orientation;

                break;
            }
        } else {
            if(i > 8 && buf[i]==0x00 && buf[i-1]==0x00
               && buf[i-2]==0x00 && buf[i-3]==0x01 
               && buf[i-4]==0x00 && buf[i-5]==0x03
               && buf[i-6]==0x01 && buf[i-7]==0x12)
            {
                stream >> orientation;

                break;
            }
        }
    }


    file.close();

    //kdDebug() << "Image Orientation: " << (KExifData::ImageOrientation)orientation << endl;
    
    return (KExifData::ImageOrientation)orientation;
}


#endif /* EXIFORIENTATION_P_H */

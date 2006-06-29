/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2006-06-29
 * Description : RAW file preview extraction using dcraw
 *
 * Copyright 2006 by Gilles Caulier
 *
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>

// Qt Includes.

#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kdebug.h>
#include <kprocess.h>

// Local includes.

#include "rawfiles.h"
#include "dcrawpreview.h"

namespace Digikam
{

bool DcrawPreview::loadDcrawPreview(QImage& image, const QString& path)
{
    FILE       *f;
    QByteArray  imgData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    Q_LONG      len;
    QCString    command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);

    if (!fileInfo.exists() || !rawFilesExt.upper().contains( fileInfo.extension().upper() ))
        return false;

    // Try to extract embedded thumbnail using dcraw with options:
    // -c : write to stdout
    // -e : Extract the camera-generated thumbnail, not the raw image (JPEG or a PPM file).
    // Note : this code require at least dcraw version 8.x

    command  = "dcraw -c -e ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug() << "Running dcraw command " << command << endl;

    f = popen( command.data(), "r" );

    if ( !f )
        return false;

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            kdDebug() << "Using embedded RAW preview extraction" << endl;
            return true;
        }
    }

    // In second, try to use simple RAW extraction method
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -2 : 8bit ppm output
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible

    command  = "dcraw -c -h -2 -w -a ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug() << "Running dcraw command " << command << endl;

    f = popen( command.data(), "r" );

    if ( !f )
        return false;

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            kdDebug() << "Using reduced RAW picture extraction" << endl;
            return true;
        }
    }

    return false;
}

}  // namespace Digikam

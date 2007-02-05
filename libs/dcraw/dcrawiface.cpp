/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2006-06-29
 * Description : dcraw interface
 *
 * Copyright 2006-2007 by Gilles Caulier
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

#include <kprocess.h>

// Local includes.

#include "ddebug.h"
#include "rawfiles.h"
#include "dcrawbinary.h"
#include "dcrawiface.h"

namespace Digikam
{

bool DcrawIface::loadDcrawPreview(QImage& image, const QString& path)
{
    FILE       *f=NULL;
    QByteArray  imgData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    Q_LONG      len;
    QCString    command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.extension(false).upper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.upper().contains(ext))
        return false;

    // Try to extract embedded thumbnail using dcraw with options:
    // -c : write to stdout
    // -e : Extract the camera-generated thumbnail, not the raw image (JPEG or a PPM file).
    // Note : this code require at least dcraw version 8.x

    command  = DcrawBinary::instance()->path();
    command += " -c -e ";
    command += QFile::encodeName( KProcess::quote( path ) );
    DDebug() << "Running RAW decoding command " << command << endl;

    f = popen( command.data(), "r" );

    if ( f == NULL )
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
            DDebug() << "Using embedded RAW preview extraction" << endl;
            return true;
        }
    }

    // In second, try to use simple RAW extraction method in 8 bits ppm output.
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible

    f=NULL;
    command  = DcrawBinary::instance()->path();
    command += " -c -h -w -a ";
    command += QFile::encodeName( KProcess::quote( path ) );
    DDebug() << "Running RAW decoding command " << command << endl;

    f = popen( command.data(), "r" );

    if ( f == NULL )
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
            DDebug() << "Using reduced RAW picture extraction" << endl;
            return true;
        }
    }

    return false;
}

bool DcrawIface::rawFileIdentify(DcrawInfoContainer& identify, const QString& path)
{
    FILE       *f=NULL;
    QByteArray  txtData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    Q_LONG      len;
    QCString    command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.extension(false).upper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.upper().contains(ext))
        return false;

    // Try to get camera maker/model using dcraw with options:
    // -c : write to stdout
    // -i : identify files without decoding them.
    // -v : verbose mode.

    command  = DcrawBinary::instance()->path();
    command += " -c -i -v ";
    command += QFile::encodeName( KProcess::quote( path ) );
    DDebug() << "Running RAW decoding command " << command << endl;

    f = popen( command.data(), "r" );

    if ( f == NULL )
    {
        identify = DcrawInfoContainer();
        return false;
    }

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            identify = DcrawInfoContainer();
            return false;
        }
        else
        {
            int oldSize = txtData.size();
            txtData.resize( txtData.size() + len );
            memcpy(txtData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );
    QString dcrawInfo(txtData);

    if ( dcrawInfo.isEmpty() )
    {
        identify = DcrawInfoContainer();
        return false;
    }

    // Extract Time Stamp.
    QString timeStampHeader("Timestamp: ");
    QString timeStamp = dcrawInfo.section('\n', 2, 2);
    timeStamp.remove(0, timeStampHeader.length());
    identify.dateTime = QDateTime::fromString(timeStamp);

    // Extract Camera Model.
    QString cameraHeader("Camera: ");
    QString camera = dcrawInfo.section('\n', 3, 3);
    camera.remove(0, cameraHeader.length());
    identify.model = camera;

    // Extract ISO Speed.
    QString isoSpeedHeader("ISO speed: ");
    QString isoSpeed = dcrawInfo.section('\n', 4, 4);
    isoSpeed.remove(0, isoSpeedHeader.length());
    identify.sensitivity = isoSpeed.toLong();

    // Extract Shutter Speed.
    QString shutterSpeedHeader("Shutter: 1/");
    QString shutterSpeed = dcrawInfo.section('\n', 5, 5);
    shutterSpeed.remove(0, shutterSpeedHeader.length());
    shutterSpeed.remove(shutterSpeed.length()-4, 4);    // remove " sec" at end of string.
    identify.exposureTime = shutterSpeed.toFloat();

    // Extract Aperture.
    QString apertureHeader("Aperture: f/");
    QString aperture = dcrawInfo.section('\n', 6, 6);
    aperture.remove(0, apertureHeader.length());
    identify.aperture = aperture.toFloat();

    // Extract Focal Length.
    QString focalLengthHeader("Focal Length: ");
    QString focalLength = dcrawInfo.section('\n', 7, 7);
    focalLength.remove(0, focalLengthHeader.length());
    focalLength.remove(focalLength.length()-3, 3);    // remove " mm" at end of string.
    identify.focalLength = focalLength.toFloat();

    return true;
}

}  // namespace Digikam

/***************************************************************************
 *   Copyright (C) 2004 by Renchi Raju                                     *
 *   renchi@pooh.tam.uiuc.edu                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qcstring.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qimage.h>

#include <kio/global.h>
#include <kinstance.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
}

#include "gpcamera.h"
#include "gpmessages.h"
#include "digikamcamera.h"

using namespace KIO;


DigikamCamera::DigikamCamera(const QCString &pool_socket, const QCString &app_socket)
    : SlaveBase("kio_digikamcamera", pool_socket, app_socket)
{
    kdDebug() << "DigikamCamera::DigikamCamera()" << endl;

    m_camera = 0;
}


DigikamCamera::~DigikamCamera()
{
    kdDebug() << "DigikamCamera::~DigikamCamera()" << endl;

    if (m_camera)
        delete m_camera;
}

void DigikamCamera::openConnection()
{
    closeConnection();

    kdDebug() << "Opening connection" << endl;

    if (!hasMetaData("model") || !hasMetaData("port") ||
        !hasMetaData("path"))
    {
        kdWarning() << "Incomplete metainfo" << endl;
        closeConnection();
        return;
    }

    connected();
    
    m_model = metaData("model");
    m_port  = metaData("port");
    m_path  = metaData("path");

    m_camera = new GPCamera(this, m_model, m_port, m_path);

    connect(GPMessages::gpMessagesWrapper(),
            SIGNAL(errorMessage(const QString&)),
            SLOT(slotErrorMsg(const QString&)));
    connect(GPMessages::gpMessagesWrapper(),
            SIGNAL(statusChanged(const QString&)),
            SLOT(slotInfoMsg(const QString&)));
    connect(GPMessages::gpMessagesWrapper(),
            SIGNAL(progressStart(int)),
            SLOT(slotProgressStart(int)));
    connect(GPMessages::gpMessagesWrapper(),
            SIGNAL(progressUpdate(int)),
            SLOT(slotProgressUpdate(int)));
}

void DigikamCamera::closeConnection()
{
    if (m_camera)
    {
        delete m_camera;
        m_camera = 0;

        GPMessages::deleteMessagesWrapper();
    }
    kdDebug() << "Closing connection" << endl;
}


void DigikamCamera::special(const QByteArray &da)
{
    /* Commands:
       1 - initialize camera
       2 - cancel current operation
       3 - download item
       4 - get camera information
    */
    
    if (da.size() < 1)
    {
        finished();
        return;
    }

    QDataStream ds(da, IO_ReadOnly);
    int cmd;
    ds >> cmd;
    
    if (cmd == 1)
    {
        infoMessage(i18n("Connecting to Camera..."));
        int result = m_camera->initialize();
        if (result != GPCamera::GPSuccess)
        {
            if (result == GPCamera::GPSetup)
                error(KIO::ERR_SERVICE_NOT_AVAILABLE,
                      i18n("Camera Model or Port not specified correctly.\n"
                           "Please run Setup"));
            else
                error(KIO::ERR_SERVICE_NOT_AVAILABLE,
                      i18n("Failed to initialize camera\n"
                           "Please ensure camera is connected properly and turned on"));
        }
    }
    else if (cmd == 2)
    {
        m_camera->cancel();
    }
    else if (cmd == 3)
    {
        QString src;
        QString dest;
        ds >> src; 
        ds >> dest; 

        KURL url(src);
        url.setProtocol("digikamcamera");
        QString folder(url.directory());
        QString name(url.fileName());

        kdDebug() << "Downloading From " << url.prettyURL() << " to "
                  << dest << endl;
        
        if (m_camera->downloadItem(folder, name, dest) != GPCamera::GPSuccess)
            error(KIO::ERR_UNKNOWN, i18n("Failed to download file %1").arg(name));
    }
    else if (cmd == 4)
    {
        infoMessage(i18n("Retrieving Camera Information..."));

        QString summary;
        QString manual;
        QString about;

        m_camera->cameraSummary(summary);
        m_camera->cameraManual(manual);
        m_camera->cameraAbout(about);

        QByteArray ba;
        QDataStream ds(ba, IO_WriteOnly);
        ds << summary;
        ds << manual;
        ds << about;

        data(ba);
    }
    else
    {
        error(KIO::ERR_UNKNOWN, i18n("Unknown Command sent to cameraclient"));
    }    

    finished();
}


void DigikamCamera::get(const KURL& url )
{
    if (metaData("thumbnail") == "1")
    {
        int size = metaData("size").toInt();
    
        if (size <= 0) 
        {
            error(KIO::ERR_INTERNAL, i18n("No or invalid size specified"));
            kdWarning() << "No or invalid size specified" << endl;
            return;
        }
        
        QImage thumb;

        m_camera->getThumbnail(url.directory(), url.fileName(), thumb);

        if (QMAX(thumb.width(),thumb.height()) != size)
            thumb = thumb.smoothScale(size, size, QImage::ScaleMin);

        if (thumb.depth() != 32)
            thumb = thumb.convertDepth(32);

        QByteArray imgData;
        QDataStream stream( imgData, IO_WriteOnly );

        QString shmid = metaData("shmid");
    
        if (shmid.isEmpty()) 
        {
            stream << thumb;
        }
        else
        {
            void *shmaddr = shmat(shmid.toInt(), 0, 0);
        
            if (shmaddr == (void *)-1)
            {
                error(KIO::ERR_INTERNAL, "Failed to attach to shared memory segment");
                kdWarning() << "Failed to attach to shared memory segment " << shmid << endl;
                return;
            }
            
            if (thumb.width() * thumb.height() > size * size)
            {
                error(KIO::ERR_INTERNAL, "Image is too big for the shared memory segment");
                kdWarning() << "Image is too big for the shared memory segment" << endl;
                shmdt((char*)shmaddr);
                return;
            }
            
            stream << thumb.width() << thumb.height() << thumb.depth();
            memcpy(shmaddr, thumb.bits(), thumb.numBytes());
            shmdt((char*)shmaddr);
        }

        data(imgData);
    }
    else if (metaData("exif") == "1")
    {
        char *edata = 0;
        int esize   = 0;

        m_camera->getExif(url.directory(), url.fileName(), &edata, esize);

        if (!edata || !esize)
        {
            error(KIO::ERR_DOES_NOT_EXIST,
                  i18n("Failed to get exif information for %1")
                  .arg(url.prettyURL()));
        }
        else
        {
            QByteArray exifData;
            QDataStream stream( exifData, IO_WriteOnly );
            stream.writeBytes( edata, esize);
            delete [] edata;

            data(exifData);
        }
    }
    
    data(QByteArray());
    finished();
}

void DigikamCamera::listDir(const KURL& url)
{
    kdDebug() << "DigikamCamera::listDir " << url << endl ;

    if (m_camera)
    {
        m_camera->getAllItemsInfo(m_path);
    }
    else
    {
        kdWarning() << "ListDir: No Camera" << endl;
    }
    
    KIO::UDSEntry entry;
    listEntry(entry, true);
    finished();
}

void DigikamCamera::stat(const KURL& url)
{
    kdDebug() << k_funcinfo << url << endl;

    if (QDir::cleanDirPath(url.path(1)) == "/")
    {
        KIO::UDSEntry entry;
        KIO::UDSAtom  atom;

        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = "/";
        entry.append(atom);

        atom.m_uds  = KIO::UDS_FILE_TYPE;
        atom.m_long = S_IFDIR;
        entry.append(atom);

        atom.m_uds = KIO::UDS_ACCESS;
        atom.m_long = S_IRUSR | S_IRGRP | S_IROTH |
                      S_IWUSR | S_IWGRP | S_IWOTH;
        entry.append(atom);

        statEntry(entry);
    
        finished();
    }
    else
    {
        error(KIO::ERR_COULD_NOT_STAT, i18n("No such folder on camera"));
        finished();
    }
}

void DigikamCamera::copy(const KURL& src, const KURL& dest,
                         int , bool )
{
    if (src.protocol() != "digikamcamera")
    {
        error(KIO::ERR_COULD_NOT_READ, i18n("File not present on camera"));
    }
    else if (dest.protocol() != "file")
    {
        error(KIO::ERR_COULD_NOT_READ, i18n("Destination file is not a local file"));
    }
    else
    {
        QString folder(src.directory());
        QString name(src.fileName());
        if (m_camera->downloadItem(folder, name, dest.path()) != GPCamera::GPSuccess)
            error(KIO::ERR_COULD_NOT_READ, i18n("Failed to download file %1").arg(name));
    }

    finished();
}

void DigikamCamera::slotErrorMsg(const QString& msg)
{
    error(KIO::ERR_UNKNOWN, msg);
}

void DigikamCamera::slotInfoMsg(const QString& msg)
{
    infoMessage(msg);
}

void DigikamCamera::slotProgressStart(int val)
{
    totalSize(val);
}

void DigikamCamera::slotProgressUpdate(int val)
{
    processedSize(val);    
}

extern "C"
{
    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_digikamcamera" );
        
        kdDebug() << "*** kio_digikamcamera Started ***" << endl;
        
        if (argc != 4) {
            kdDebug(7101) << "Usage: kio_digikamcamera  protocol domain-socket1 domain-socket2" << endl;
            exit(-1);
        }
        
        DigikamCamera slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kdDebug() << "*** kio_digikamcamera Done ***" << endl;
        return 0;
    }
}

#include "digikamcamera.moc"

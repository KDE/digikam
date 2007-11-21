/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-01-15
 * Description : digiKam KIO slave to get image thumbnails. 
 *               This kio-slave support this freedesktop 
 *               specification about thumbnails mamagement:
 *               http://jens.triq.net/thumbnail-spec
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cstring>

// Qt Includes.

#include <QByteArray>
#include <QString>
#include <QImage>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMatrix>
#include <QRegExp>
#include <QApplication>

// KDE includes.

#include <kdebug.h>
#include <kurl.h>
#include <kcomponentdata.h>
#include <kimageio.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcodecs.h>
#include <ktemporaryfile.h>
#include <kfilemetainfo.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "dimg.h"
#include "digikamthumbnail.h"
#include "digikam_export.h"

// C Ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
}

using namespace KIO;
using namespace Digikam;

kio_digikamthumbnailProtocol::kio_digikamthumbnailProtocol(int /*argc*/, char** argv) 
                            : SlaveBase("kio_digikamthumbnail", argv[2], argv[3])
{
    m_creator = new Digikam::ThumbnailCreator;
    /*
    m_argc = argc;
    m_argv = argv;
    m_app  = 0;
    */
}

kio_digikamthumbnailProtocol::~kio_digikamthumbnailProtocol()
{
}

void kio_digikamthumbnailProtocol::get(const KUrl& url )
{
    int  size =  metaData("size").toInt();
    bool exif = (metaData("exif") == "yes");

    m_creator->setThumbnailSize(size);
    m_creator->setExifRotate(exif);
    QImage img = m_creator->load(url.path(KUrl::RemoveTrailingSlash));

    if (img.isNull())
    {
        error(KIO::ERR_INTERNAL, m_creator->errorString());
        return;
    }

    QByteArray imgData;
    QDataStream stream(&imgData, QIODevice::WriteOnly);

    const QString shmid = metaData("shmid");
    if (shmid.isEmpty())
    {
        stream << img;
    }
    else
    {
        void *shmaddr = shmat(shmid.toInt(), 0, 0);

        if (shmaddr == (void *)-1)
        {
            error(KIO::ERR_INTERNAL, "Failed to attach to shared memory segment " + shmid);
            kWarning() << "Failed to attach to shared memory segment " << shmid;
            return;
        }

        if (img.width() * img.height() > m_creator->cachedSize() * m_creator->cachedSize())
        {
            error(KIO::ERR_INTERNAL, "Image is too big for the shared memory segment");
            kWarning() << "Image is too big for the shared memory segment";
            shmdt((char*)shmaddr);
            return;
        }

        // NOTE: KDE4PORT: Qt4::QImage API has change. We will use "format" instead "depth".
        stream << img.width() << img.height() << (int)img.format();
        memcpy(shmaddr, img.bits(), img.numBytes());
        shmdt((char*)shmaddr);
    }

    data(imgData);
    finished();
}

#if 0

This is code that has not been moved to ThumbnailCreator.
We are now using PreviewJob directly from ThumbnailJob in digikam.
There, we dont need to do hacks with KApplication

#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kmimetype.h>
#include <kio/global.h>
#include <kio/thumbcreator.h>

// -- Load using KDE API ---------------------------------------------------------------------

bool kio_digikamthumbnailProtocol::loadKDEThumbCreator(QImage& image, const QString& path)
{
    // this sucks royally. some of the thumbcreators need an instance of
    // app running so that they can use pixmap. till they get their 
    // code fixed, we will have to create a qapp instance.
    if (!m_app)
        m_app = new QApplication(m_argc, m_argv);

    QString mimeType = KMimeType::findByUrl(path)->name();
    if (mimeType.isEmpty())
    {
        kDebug() << "Mimetype not found";
        return false;
    }

    QString mimeTypeAlt = mimeType.replace(QRegExp("/.*"), "/*");

    QString plugin;

    KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
    {
        QStringList mimeTypes = (*it)->property("MimeTypes").toStringList();
        for (QStringList::ConstIterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
        {
            if  ((*mt) == mimeType || (*mt) == mimeTypeAlt)
            {
                plugin = (*it)->library();
                break;
            }
        }

        if (!plugin.isEmpty())
            break;
    }

    if (plugin.isEmpty())
    {
        kDebug() << "No relevant plugin found ";
        return false;
    }

    KLibrary *library = KLibLoader::self()->library(QFile::encodeName(plugin));
    if (!library)
    {
        kDebug() << "Plugin library not found " << plugin;
        return false;
    }

    ThumbCreator *creator = 0;
    newCreator create = (newCreator)library->resolveSymbol("new_creator");
    if (create)
        creator = create();

    if (!creator)
    {
        kDebug() << "Cannot load ThumbCreator " << plugin;
        return false;
    }

    if (!creator->create(path, m_cachedSize, m_cachedSize, image))
    {
        kDebug() << "Cannot create thumbnail for " << path;
        delete creator;
        return false;
    }  

    delete creator;
    return true;
}

#endif


// -- KIO slave registration ---------------------------------------------------------------------

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalog("digikam");

        // The creation of a QCoreApplication is necessary here for only one reason:
        // The Qt image plugins are not found without it.
        // We need Qt JPG support in ThumbCreator, KDcraw to load the embedded preview.
        // If anyone knows if the loading of plugins can be achieved with less
        // than creating a QCoreApplication, please change it.
        QCoreApplication app(argc, argv);

        KComponentData componentData( "kio_digikamthumbnail" );
        ( void ) KGlobal::locale();

        if (argc != 4) 
        {
            kDebug() << "Usage: kio_digikamthumbnail  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamthumbnailProtocol slave(argc, argv);
        slave.dispatchLoop();

        return 0;
    }
}

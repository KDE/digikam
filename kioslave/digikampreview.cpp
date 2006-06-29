/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2006-19-06
 * Description : digiKam KIO slave to extract image preview.
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

// C++ includes.

#include <cstdlib>
#include <cstdio>

// Qt Includes.

#include <qstring.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qwmatrix.h>

// KDE includes.

#include <kdebug.h>
#include <kurl.h>
#include <kinstance.h>
#include <kimageio.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <kio/global.h>
#include <kfilemetainfo.h>

// C Ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
}

// Local includes

#include "dcrawpreview.h"
#include "dmetadata.h"
#include "digikampreview.h"
#include "digikam_export.h"

using namespace KIO;
using namespace Digikam;

kio_digikampreviewProtocol::kio_digikampreviewProtocol(int /*argc*/, char** argv) 
                          : SlaveBase("kio_digikampreview", argv[2], argv[3])
{
}

kio_digikampreviewProtocol::~kio_digikampreviewProtocol()
{
}

void kio_digikampreviewProtocol::get(const KURL& url )
{
    int  size =  metaData("size").toInt();
    bool exif = (metaData("exif") == "yes");

    QImage img;

    // stat the original file
    struct stat st;
    if (::stat(QFile::encodeName(url.path(-1)), &st) != 0)
    {
        error(KIO::ERR_INTERNAL, i18n("File does not exist"));
        return;
    }

    // -- Get the image preview --------------------------------
    // In first, we trying to load with dcraw : RAW files.
    if ( !DcrawPreview::loadDcrawPreview(img, url.path()) )
    {
        // Try to extract Exif/Iptc preview.
        if ( !loadImagePreview(img, url.path()) )
            // Try to load with Qt/KDE.
            img.load(url.path());
    }

    if (img.isNull())
    {
        error(KIO::ERR_INTERNAL, i18n("Cannot extract preview for %1")
              .arg(url.prettyURL()));
        kdWarning() << "Cannot extract preview for " << url.path() << endl;
        return;
    }

    if (img.depth() != 32)
        img = img.convertDepth(32);

    if (exif)
        exifRotate(url.path(), img);

    img = img.smoothScale(size, size, QImage::ScaleMin);

    QByteArray imgData;
    QDataStream stream( imgData, IO_WriteOnly );

    QString shmid = metaData("shmid");

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
            kdWarning() << "Failed to attach to shared memory segment " << shmid << endl;
            return;
        }

        if (img.width() * img.height() > size * size)
        {
            error(KIO::ERR_INTERNAL, "Image is too big for the shared memory segment");
            kdWarning() << "Image is too big for the shared memory segment" << endl;
            shmdt((char*)shmaddr);
            return;
        }

        stream << img.width() << img.height() << img.depth();
        memcpy(shmaddr, img.bits(), img.numBytes());
        shmdt((char*)shmaddr);
    }

    data(imgData);
    finished();
}

void kio_digikampreviewProtocol::exifRotate(const QString& filePath, QImage& thumb)
{
    // Check if the file is an JPEG image
    KFileMetaInfo metaInfo(filePath, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.isValid())
    {
        if (metaInfo.mimeType() == "image/jpeg" &&
            metaInfo.containsGroup("Jpeg EXIF Data"))
        {
            // Rotate thumbnail from JPEG files based on EXIF rotate tag

            QWMatrix matrix;
            DMetadata metadata(filePath);
            DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

            bool doXform = (orientation != DMetadata::ORIENTATION_NORMAL &&
                            orientation != DMetadata::ORIENTATION_UNSPECIFIED);

            switch (orientation) 
            {
                case DMetadata::ORIENTATION_NORMAL:
                case DMetadata::ORIENTATION_UNSPECIFIED:
                    break;

                case DMetadata::ORIENTATION_HFLIP:
                    matrix.scale(-1, 1);
                    break;

                case DMetadata::ORIENTATION_ROT_180:
                    matrix.rotate(180);
                    break;

                case DMetadata::ORIENTATION_VFLIP:
                    matrix.scale(1, -1);
                    break;

                case DMetadata::ORIENTATION_ROT_90_HFLIP:
                    matrix.scale(-1, 1);
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_90:
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_90_VFLIP:
                    matrix.scale(1, -1);
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_270:
                    matrix.rotate(270);
                    break;
            }

            //transform accordingly
            if ( doXform )
                thumb = thumb.xForm( matrix );
        }
    }
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool kio_digikampreviewProtocol::loadImagePreview(QImage& image, const QString& path)
{
    DMetadata metadata(path);
    if (metadata.getImagePreview(image))
    {
        kdDebug() << "Use Exif/Iptc preview extraction. Size of image: " 
                  << image.width() << "x" << image.height() << endl;
        return true;
    }

    return false;
}

// -- KIO slave registration ---------------------------------------------------------------------

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikampreview" );
        ( void ) KGlobal::locale();

        if (argc != 4) 
        {
            kdDebug() << "Usage: kio_digikampreview  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        KImageIO::registerFormats();

        kio_digikampreviewProtocol slave(argc, argv);
        slave.dispatchLoop();

        return 0;
    }
}

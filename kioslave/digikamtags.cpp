/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-09
 * Description :
 *
 * Copyright 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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

#include <digikam_export.h>

#include <kinstance.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kfilemetainfo.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qdir.h>
#include <qregexp.h>

extern "C"
{
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
}

#include "databaseaccess.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "imagelisterreceiver.h"
#include "digikamtags.h"

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QCString &pool_socket,
                                                 const QCString &app_socket)
    : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
}

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    KURL    kurl;
    QString filter;
    int     getDimensions;

    QDataStream ds(data, IO_ReadOnly);
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    Digikam::ImageLister lister;
    // send data every 200 images to be more responsive
    Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
    lister.list(&receiver, kurl, filter, getDimensions);
    // send rest
    receiver.sendData();

    finished();
}

/* KIO slave registration */

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamtags" );
        ( void ) KGlobal::locale();

        kdDebug() << "*** kio_digikamtag started ***" << endl;

        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamtags  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamtagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kdDebug() << "*** kio_digikamtags finished ***" << endl;
        return 0;
    }
}


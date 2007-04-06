/* ============================================================
 * File  : digikamdates.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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

#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kdebug.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qbuffer.h>

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

#include "imagelister.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "namefilter.h"
#include "digikamdates.h"

kio_digikamdates::kio_digikamdates(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamdates", pool_socket, app_socket)
{
}

kio_digikamdates::~kio_digikamdates()
{
}

void kio_digikamdates::special(const QByteArray& data)
{
    bool folders = (metaData("folders") == "yes");

    KURL    kurl;
    QString url;
    QString filter;
    int     getDimensions;

    QDataStream ds(data, IO_ReadOnly);
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    url = kurl.path();

    if (folders)
    {
        Digikam::NameFilter nameFilter(filter);
        QByteArray  ba;

        typedef QPair<int, int> YearMonth;
        QMap<YearMonth, bool> yearMonthMap;

        QValueList<QPair<QString, QDateTime> > images;
        {
            Digikam::DatabaseAccess access;
            images = access.db()->getItemsAndDate();
        }

        for ( QValueList<QPair<QString, QDateTime> >::iterator it = images.begin(); it != images.end(); ++it)
        {
            if ( !nameFilter.matches((*it).first) )
                continue;

            if ( !yearMonthMap.contains(YearMonth((*it).second.date().year(), (*it).second.date().month())) )
            {
                yearMonthMap.insert( YearMonth( (*it).second.date().year(), (*it).second.date().month() ), true );
            }
        }

        QDataStream os(ba, IO_WriteOnly);

        int year, month;
        for ( QMap<YearMonth, bool>::iterator it = yearMonthMap.begin();
              it != yearMonthMap.end(); ++it )
        {
            year  = it.key().first;
            month = it.key().second;

            QDate date( year,  month,  1 );
            os << date;
        }

        SlaveBase::data(ba);
    }
    else
    {
        Digikam::ImageLister lister;
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.list(&receiver, kurl, filter, getDimensions);
        // send rest
        receiver.sendData();
    }

    finished();
}

/* KIO slave registration */

extern "C"  
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamdates" );
        KGlobal::locale();

        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamdates  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamdates slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}


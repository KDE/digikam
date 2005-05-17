/* ============================================================
 * File  : dio.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-17
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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


#include <kprotocolinfo.h>
#include <kglobalsettings.h>

#include "albumsettings.h"
#include "albummanager.h"
#include "albumlister.h"
#include "dio.h"
#include "dio_p.h"

namespace DIO
{

KIO::Job* copy(const KURL& src, const KURL& dest)
{
    KIO::Job* job = KIO::copy(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* copy(const KURL::List& srcList, const KURL& dest)
{
    KIO::Job* job = KIO::copy(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KURL& src, const KURL& dest)
{
    KIO::Job* job = KIO::move(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KURL::List& srcList, const KURL& dest)
{
    KIO::Job* job = KIO::move(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* del(const KURL& src)
{
    KIO::Job* job = 0;
    
    if (AlbumSettings::instance()->getUseTrash())
    {
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }
        
        job = KIO::move( src, dest );
    }   
    else
    {
        job = KIO::del(src);
    }
    
    new Watch(job);
    return job;
}

KIO::Job* del(const KURL::List& srcList)
{
    KIO::Job* job = 0;
    
    if (AlbumSettings::instance()->getUseTrash())
    {
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }
        
        job = KIO::move( srcList, dest );
    }   
    else
    {
        job = KIO::del(srcList);
    }
    
    new Watch(job);
    return job;
}

Watch::Watch(KIO::Job* job)
{
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotDone(KIO::Job*)));
}

void Watch::slotDone(KIO::Job*)
{
    //TODO: AlbumManager::instance()->refresh();
    AlbumLister::instance()->refresh();
    delete this;
}

}

#include "dio_p.moc"

/* ============================================================
 * File  : syncjob.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-10-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
 *
 * Concept copied from kdelibs/kio/kio/netaccess.h/cpp
 *   This file is part of the KDE libraries
 *   Copyright (C) 1997 Torben Weis (weis@kde.org)
 *   Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
 *   Copyright (C) 1999 David Faure (faure@kde.org)
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kio/job.h>
#include <kglobalsettings.h>

#include <qapplication.h>

#include "albumsettings.h"
#include "syncjob.h"

QString* SyncJob::lastErrorMsg_  = 0;
int      SyncJob::lastErrorCode_ = 0;

bool SyncJob::userDelete(const KURL::List& urls)
{
    if (AlbumSettings::instance()->getUseTrash())
        return trash(urls);
    else
        return del(urls);
}

bool SyncJob::del(const KURL::List& urls)
{
    SyncJob sj;
    return sj.delPriv(urls);    
}

bool SyncJob::trash(const KURL::List& urls)
{
    SyncJob sj;
    return sj.trashPriv(urls);    
}

SyncJob::SyncJob()
{
}

SyncJob::~SyncJob()
{
}

bool SyncJob::delPriv(const KURL::List& urls)
{
    success_ = true;

    KIO::Job* job = KIO::del( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             SLOT(slotResult( KIO::Job*)) );

    enter_loop();
    return success_;
}

bool SyncJob::trashPriv(const KURL::List& urls)
{
    success_ = true;

    KURL dest(KGlobalSettings::trashPath());
    
    KIO::Job* job = KIO::move( urls, dest );
    connect( job, SIGNAL(result( KIO::Job* )),
             SLOT(slotResult( KIO::Job*)) );

    enter_loop();
    return success_;
}

void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void SyncJob::enter_loop()
{
    QWidget dummy(0,0,WType_Dialog | WShowModal);
    dummy.setFocusPolicy( QWidget::NoFocus );
    qt_enter_modal(&dummy);
    qApp->enter_loop();
    qt_leave_modal(&dummy);
}

void SyncJob::slotResult( KIO::Job * job )
{
    lastErrorCode_ = job->error();
    success_ = !(lastErrorCode_);
    if ( !success_ )
    {
        if ( !lastErrorMsg_ )
            lastErrorMsg_ = new QString;
        *lastErrorMsg_ = job->errorString();
    }
    qApp->exit_loop();
}

QString SyncJob::lastErrorMsg()
{
    return (lastErrorMsg_ ? *lastErrorMsg_ : QString::null);
}

int SyncJob::lastErrorCode()
{
    return lastErrorCode_;
}

#include "syncjob.moc"

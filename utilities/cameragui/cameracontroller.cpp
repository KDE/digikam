/* ============================================================
 * File  : cameracontroller.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-17
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kio/scheduler.h>
#include <kio/slave.h>
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

#include <qstring.h>
#include <qtimer.h>
#include <qdatastream.h>
#include <qguardedptr.h>

#include "camerathumbjob.h"
#include "cameradownloaddlg.h"
#include "cameracontroller.h"

class CameraControllerPriv
{
public:

    QWidget *parent;
    
    QString model;
    QString port;
    QString path;

    KIO::Slave             *slave;
    QGuardedPtr<KIO::Job>   job;

    bool slaveConnected;
    bool cameraConnected;

    CameraController::State state;

    KFileItemList itemList;
};
    

CameraController::CameraController(QWidget *parent, const QString& model,
                                   const QString& port, const QString& path)
{
    d = new CameraControllerPriv;
    d->parent = parent;
    d->model  = model;
    d->port   = port;
    d->path   = path;
    d->state  = ST_NONE;
    d->slaveConnected   = false;
    d->cameraConnected  = false;
    d->itemList.setAutoDelete(true);

    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");

    KIO::MetaData m;
    m["model"] = model;
    m["port"]  = port;
    m["path"]  = path;

    d->slave = KIO::Scheduler::getConnectedSlave(url, m);
    if (!d->slave)
    {
        QTimer::singleShot(0, this, SLOT(slotSlaveConnectFailed()));
        return;
    }
    
    KIO::Scheduler::
        connect(SIGNAL(slaveError(KIO::Slave *, int, const QString&)),
                this, SLOT(slotSlaveError(KIO::Slave *, int, const QString&)));
    KIO::Scheduler::
        connect(SIGNAL(slaveConnected(KIO::Slave *)),
                this, SLOT(slotSlaveConnected(KIO::Slave *)));
}

CameraController::~CameraController()
{
    emit signalClear();

    if (!d->job.isNull())
    {
        delete d->job;
    }
    
    if (d->slave)
    {
        KIO::Scheduler::disconnectSlave(d->slave);
    }
    delete d;
}

void CameraController::downloadAll(const QString& destFolder)
{
    CameraDownloadDlg dlg(d->parent, d->slave, d->itemList,
                          destFolder);
    dlg.exec();
}

void CameraController::downloadSel(const KFileItemList& items,
                                   const QString& destFolder)
{
    CameraDownloadDlg dlg(d->parent, d->slave, items,
                          destFolder);
    dlg.exec();
}

void CameraController::deleteAll()
{
    
}

void CameraController::deleteSel(const KFileItemList& )
{
    
}

void CameraController::slotCancel()
{
    if (d->job.isNull())
        return;

    /* don't kill jobs. the slave gets killed too.
       send a cmd cancel to the remote kioslave */
    
    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << 2;
    KIO::SimpleJob* job = new KIO::SimpleJob(url, KIO::CMD_SPECIAL,
                                             ba, false);
    KIO::Scheduler::assignJobToSlave(d->slave, job);
    
    d->state = ST_NONE;
    emit signalInfoPercent(0);
    emit signalInfoMessage(i18n("Ready"));
    emit signalBusy(false);
    delete d->job;
}

void CameraController::slotSlaveConnected(KIO::Slave* slave)
{
    if (slave == d->slave)
    {
        d->slaveConnected = true;

        /* Now connect to the camera */

        KURL url;
        url.setProtocol("digikamcamera");
        url.setPath("/");

        d->state = ST_CONNECT;

        QByteArray ba;
        QDataStream ds(ba, IO_WriteOnly);
        ds << 1;
        KIO::SimpleJob* job = new KIO::SimpleJob(url, KIO::CMD_SPECIAL,
                                                 ba, false);
        d->job = job;

        connect(job, SIGNAL(result(KIO::Job*)),
                SLOT(slotResult(KIO::Job*)));
        connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
                SLOT(slotInfoMessage(KIO::Job*, const QString&)));
        connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
                SLOT(slotPercent(KIO::Job*, unsigned long)));

        KIO::Scheduler::assignJobToSlave(d->slave, job);

        emit signalBusy(true);
    }
}

void CameraController::slotSlaveConnectFailed()
{
    emit signalFatal(i18n("Failed to Initialize Camera Client.\n"
                          "Please check your installation"));
}

void CameraController::slotSlaveError(KIO::Slave *slave, int error,
                                      const QString &errorMsg)
{
    if (slave == d->slave)
    {
        switch (error)
        {
        case(KIO::ERR_CONNECTION_BROKEN):
        {
            KIO::Scheduler::disconnectSlave(d->slave);
        }
        case(KIO::ERR_SERVICE_NOT_AVAILABLE):
        {
            emit signalFatal(errorMsg);
            break;
        }
        default:
            kdWarning() << k_funcinfo << errorMsg << endl;
        }
    }
}

void CameraController::slotResult(KIO::Job *job)
{
    if (job->error())
    {
        if (d->state == ST_CONNECT)
        {
            emit signalFatal(QString("Failed to connect to camera"));
        }
        else
        {
            job->showErrorDialog();
        }
        emit signalInfoPercent(0);
        emit signalInfoMessage(i18n("Ready"));
        emit signalBusy(false);
        return;
    }

    if (d->state == ST_CONNECT)
    {
        /* connected. start listing */
        d->cameraConnected = true;
        QTimer::singleShot(0, this, SLOT(slotList()));
    }

    if (d->state == ST_LIST)
    {
        /* listing finished. start getting thumbnails */
        QTimer::singleShot(0, this, SLOT(slotThumbnails()));
    }

    d->state = ST_NONE;
    emit signalInfoPercent(0);
    emit signalInfoMessage(i18n("Ready"));
    emit signalBusy(false);
}

void CameraController::slotInfoMessage(KIO::Job *, const QString &msg)
{
    emit signalInfoMessage(msg);
}

void CameraController::slotPercent(KIO::Job *, unsigned long percent)
{
    emit signalInfoPercent((int)percent);
}

void CameraController::slotList()
{
    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");
        
    d->state = ST_LIST;
    emit signalBusy(true);
        
    KIO::ListJob* job = KIO::listDir(url, false);
    d->job = job;

    connect(job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
            SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)));
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
            SLOT(slotInfoMessage(KIO::Job*, const QString&)));
    connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
            SLOT(slotPercent(KIO::Job*, unsigned long)));

    KIO::Scheduler::assignJobToSlave(d->slave, job);
}

void CameraController::slotEntries(KIO::Job* job, const KIO::UDSEntryList& entryList)
{
    KFileItemList items;
    
    for (KIO::UDSEntryList::const_iterator it = entryList.begin();
         it != entryList.end(); ++it)
    {
        KFileItem *fileItem = new KFileItem(*it, ((KIO::ListJob*)job)->url());
        items.append(fileItem);
        d->itemList.append(fileItem);
    }

    emit signalNewItems(items);
}

void CameraController::slotThumbnails()
{
    if (d->itemList.isEmpty())
        return;

    d->state = ST_THUMBNAIL;
    emit signalBusy(true);

    CameraThumbJob *job = new CameraThumbJob(d->slave, d->itemList, 100);
    d->job = job;

    connect(job, SIGNAL(signalThumbnail(const KFileItem*, const QPixmap&)),
            SIGNAL(signalThumbnail(const KFileItem*, const QPixmap&)));
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
            SLOT(slotInfoMessage(KIO::Job*, const QString&)));
    connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
            SLOT(slotPercent(KIO::Job*, unsigned long)));
}

#include "cameracontroller.moc"

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
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qstring.h>
#include <qtimer.h>
#include <qdatastream.h>
#include <qguardedptr.h>

#include <libkexif/kexif.h>
#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

#include <imagewindow.h>
#include "camerathumbjob.h"
#include "cameradownloaddlg.h"
#include "camerainfodialog.h"
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

    // ref the slave so that scheduler doesn't kill off the slave
    // while we are still holding a pointer to it
    d->slave->ref();
    
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

        // deref the slave so that it dies on its own
        d->slave->deref();
    }
    delete d;
}

void CameraController::downloadAll(const QString& destFolder)
{
    slotCancel();

    bool slaveDied;
    
    CameraDownloadDlg dlg(d->parent, d->slave, d->itemList,
                          destFolder, &slaveDied);
    dlg.exec();

    if (slaveDied)
    {
        QTimer::singleShot(0, this, SLOT(slotSlaveDied()));
    }
}

void CameraController::downloadSel(const KFileItemList& items,
                                   const QString& destFolder)
{
    slotCancel();

    bool slaveDied;

    CameraDownloadDlg dlg(d->parent, d->slave, items,
                          destFolder, &slaveDied);
    connect(&dlg, SIGNAL(signalSlaveDied()),
            SLOT(slotSlaveDied()));
    dlg.exec();

    if (slaveDied)
    {
        QTimer::singleShot(0, this, SLOT(slotSlaveDied()));
    }
}

void CameraController::deleteAll()
{
    
}

void CameraController::deleteSel(const KFileItemList& )
{
    
}

void CameraController::openFile(const KFileItem* item)
{
    if (!item)
        return;

    slotCancel();

    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << item->url() << KURL(locateLocal("tmp", "digikamcamera.tmp"))
       << -1 << (Q_INT8) true;

    KIO::SimpleJob* job = new KIO::SimpleJob(url, KIO::CMD_COPY, ba, false);
    KIO::Scheduler::assignJobToSlave(d->slave, job);

    job->setWindow(d->parent);
    d->job = job;
    d->state = ST_OPEN;

    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
            SLOT(slotInfoMessage(KIO::Job*, const QString&)));
    connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
            SLOT(slotPercent(KIO::Job*, unsigned long)));

    emit signalBusy(true);
}

void CameraController::getExif(const KFileItem* item)
{
    if (!item)
        return;

    slotCancel();
    
    KIO::TransferJob *job = KIO::get(item->url(), false, false);
    KIO::Scheduler::assignJobToSlave(d->slave, job);

    job->setWindow(d->parent);
    job->addMetaData("exif", "1");
    d->job = job;
    d->state = ST_EXIF;

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotExifData(KIO::Job *, const QByteArray &)));
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
            SLOT(slotInfoMessage(KIO::Job*, const QString&)));
    connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
            SLOT(slotPercent(KIO::Job*, unsigned long)));

    emit signalBusy(true);
}

void CameraController::slotCameraInfo()
{
    slotCancel();

    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << 4;
    KIO::TransferJob* job = new KIO::TransferJob(url, KIO::CMD_SPECIAL,
                                                 ba, QByteArray(), false);
    KIO::Scheduler::assignJobToSlave(d->slave, job);
    job->setWindow(d->parent);
    d->job = job;
    d->state = ST_INFO;

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotInfoData(KIO::Job *, const QByteArray &)));
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
            SLOT(slotInfoMessage(KIO::Job*, const QString&)));
    connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
            SLOT(slotPercent(KIO::Job*, unsigned long)));

    emit signalBusy(true);
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
    job->setWindow(d->parent);
    KIO::Scheduler::assignJobToSlave(d->slave, job);
    
    d->state = ST_NONE;
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
        job->setWindow(d->parent);
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

void CameraController::slotSlaveDied()
{
    emit signalFatal(i18n("The connection to the camera is lost. "
                          "Please try again. If this problem persists, "
                          "please contact %1 to report this problem")
                     .arg("digikam-users@list.sourceforge.net"));
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
        case(KIO::ERR_SLAVE_DIED):
        {
            slotSlaveDied();
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
        if (job->error() == KIO::ERR_SLAVE_DIED)
        {
            slotSlaveDied();
            return;            
        }
        
        if (d->state == ST_CONNECT)
        {
            emit signalFatal(QString("Failed to connect to camera"));
        }
        else
        {
            job->showErrorDialog(d->parent);
        }
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
    if (d->state == ST_OPEN)
    {
        KURL url(locateLocal("tmp", "digikamcamera.tmp"));
        KURL::List urlList;
        urlList << url;

        ImageWindow *im = ImageWindow::instance();
        im->loadURL(urlList, url, d->model, false);
        if (im->isHidden())
            im->show();
        else
            im->raise();
        im->setFocus();
    }

    d->state = ST_NONE;
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
    job->setWindow(d->parent);
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
    job->setWindow(d->parent);
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

void CameraController::slotExifData(KIO::Job *job, const QByteArray &data)
{
    if (data.isEmpty())
        return;

    KURL url(((KIO::SimpleJob*)job)->url());
    
    uint  esize  = 0;
    char* edata = 0;
    QDataStream ds(data, IO_ReadOnly);
    ds.readBytes(edata, esize);
    if (!edata || !esize)
    {
        KMessageBox::error( d->parent,
                            i18n("No Exif information available for %1")
                            .arg(url.prettyURL()));
        return;
    }
    
    KExif exif(d->parent);
    exif.loadData(url.fileName(), edata, esize);
    delete [] edata;

    exif.exec();
}

void CameraController::slotInfoData(KIO::Job *, const QByteArray &data)
{
    if (data.isEmpty())
        return;

    QString summary;
    QString manual;
    QString about;

    QDataStream ds(data, IO_ReadOnly);
    ds >> summary;
    ds >> manual;
    ds >> about;

    CameraInfoDialog dlg(summary, manual, about);
    dlg.exec();
}

#include "cameracontroller.moc"

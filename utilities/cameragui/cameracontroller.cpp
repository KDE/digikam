/* ============================================================
 * File  : cameracontroller.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-17
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

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdeepcopy.h>
#include <qvariant.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtimer.h>

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/renamedlg.h>
#include <kdebug.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}    

#include "gpcamera.h"
#include "mtqueue.h"
#include "cameracontroller.h"

class CameraThread;

class CameraCommand
{
public:

    enum Action
    {
        gp_none = 0,
        gp_connect,
        gp_cancel,
        gp_listfolders,
        gp_listfiles,
        gp_download,
        gp_delete,
        gp_thumbnail,
        gp_exif
    };
    
    Action                 action;
    QMap<QString,QVariant> map; 
};    

class CameraEvent : public QCustomEvent
{
public:

    enum State
    {
        gp_connected = 0,
        gp_busy,
        gp_listedfolders,
        gp_listedfiles,
        gp_downloaded,
        gp_downloadFailed,
        gp_deleted,
        gp_deleteFailed,
        gp_thumbnailed,
        gp_infomsg,
        gp_errormsg
    };
    
    CameraEvent(State state) :
        QCustomEvent(QEvent::User+state)
        {}

    bool                   result;
    QString                msg;
    QMap<QString,QVariant> map; 
};
    
class CameraControllerPriv
{
public:

    QWidget*               parent;
    CameraThread*          thread;
    GPCamera*              camera;
    MTQueue<CameraCommand> cmdQueue;
    QTimer*                timer;
    bool                   close;

    bool                   overwriteAll;
    bool                   skipAll;
    int                    downloadTotal;
};

class CameraThread : public QThread
{
public:

    CameraThread(CameraController* controller);
    ~CameraThread();

    void sendBusy(bool busy);
    void sendError(const QString& msg);
    void sendInfo(const QString& msg);
    
protected:

    void run();

private:
    
    CameraControllerPriv* d;
    QObject*              parent;
};

CameraThread::CameraThread(CameraController* controller)
    : d(controller->d), parent(controller)
{
}

CameraThread::~CameraThread()
{
}

void CameraThread::run()
{
    if (d->close)
        return;

    sendBusy(true);


    CameraCommand* cmd = d->cmdQueue.dequeue();
    if (cmd)
    {

        switch (cmd->action)
        {
        case(CameraCommand::gp_connect):
        {
            sendInfo(i18n("Connecting to camera ..."));
        
            bool result = d->camera->connect();

            CameraEvent* event = new CameraEvent(CameraEvent::gp_connected);
            event->result = result;
            QApplication::postEvent(parent, event);

            if (result)
                sendInfo(i18n("Connection established"));
            else
                sendInfo(i18n("Connection failed"));

            break;
        }
        case(CameraCommand::gp_listfolders):
        {
            sendInfo(i18n("Listing folders ..."));

            QStringList folderList;
            folderList.append(d->camera->path());
            d->camera->getAllFolders(d->camera->path(), folderList);

            CameraEvent* event = new CameraEvent(CameraEvent::gp_listedfolders);
            event->map.insert("folders", QVariant(folderList));
            QApplication::postEvent(parent, event);
        
            sendInfo(i18n("Finished listing folders ..."));

            break;
        }
        case(CameraCommand::gp_listfiles):
        {
            QString folder = cmd->map["folder"].asString();
            
            sendInfo(i18n("Listing files in %1 ...")
                     .arg(folder));

            GPItemInfoList itemsList;
            if (!d->camera->getItemsInfoList(folder, itemsList))
            {
                sendError(i18n("Failed to list files in %1")
                          .arg(folder));
            }

            if (!itemsList.isEmpty())
            {
                CameraEvent* event = new CameraEvent(CameraEvent::gp_listedfiles);
                event->map.insert("folder", QVariant(folder));
                
                QByteArray  ba;
                QDataStream ds(ba, IO_WriteOnly);
                ds << itemsList;
                
                event->map.insert("files", QVariant(ba));
                QApplication::postEvent(parent, event);
            }

            sendInfo(i18n("Finished listing files in %1")
                     .arg(folder));
            
            break;
        }
        case(CameraCommand::gp_thumbnail):
        {
            QString folder = cmd->map["folder"].asString();
            QString file   = cmd->map["file"].asString();

            sendInfo(i18n("Getting thumbnail for %1/%2 ...")
                     .arg(folder)
                     .arg(file));

            QImage thumbnail;
            d->camera->getThumbnail(folder, file,
                                    thumbnail);

            if (!thumbnail.isNull())
                thumbnail = thumbnail.smoothScale(128,128,QImage::ScaleMin);
        
            CameraEvent* event = new CameraEvent(CameraEvent::gp_thumbnailed);
            event->map.insert("folder", QVariant(folder));
            event->map.insert("file", QVariant(file));
            event->map.insert("thumbnail", QVariant(thumbnail));
            QApplication::postEvent(parent, event);

            break;
        }
        case(CameraCommand::gp_download):
        {
            QString folder = cmd->map["folder"].asString();
            QString file   = cmd->map["file"].asString();
            QString dest   = cmd->map["dest"].asString();

            sendInfo(i18n("Downloading file %1 ...")
                     .arg(file));

            bool result = d->camera->downloadItem(folder, file, dest);

            if (result)
            {
                CameraEvent* event = new CameraEvent(CameraEvent::gp_downloaded);
                event->map.insert("folder", QVariant(folder));
                event->map.insert("file", QVariant(file));
                event->map.insert("dest", QVariant(dest));
                QApplication::postEvent(parent, event);
            }
            else
            {
                CameraEvent* event = new CameraEvent(CameraEvent::gp_downloadFailed);
                event->map.insert("folder", QVariant(folder));
                event->map.insert("file", QVariant(file));
                event->map.insert("dest", QVariant(dest));
                QApplication::postEvent(parent, event);
            }                
            break;
        }
        case(CameraCommand::gp_delete):
        {
            QString folder = cmd->map["folder"].asString();
            QString file   = cmd->map["file"].asString();

            sendInfo(i18n("Deleting file %1 ...")
                     .arg(file));

            bool result = d->camera->deleteItem(folder, file);

            if (result)
            {
                CameraEvent* event = new CameraEvent(CameraEvent::gp_deleted);
                event->map.insert("folder", QVariant(folder));
                event->map.insert("file", QVariant(file));
                QApplication::postEvent(parent, event);
            }
            else
            {
                CameraEvent* event = new CameraEvent(CameraEvent::gp_deleteFailed);
                event->map.insert("folder", QVariant(folder));
                event->map.insert("file", QVariant(file));
                QApplication::postEvent(parent, event);
            }                
            break;
        }
        default:
            kdWarning() << k_funcinfo << " unknown action specified" << endl;
        }    

        delete cmd;

    }

    sendBusy(false);
}

void CameraThread::sendBusy(bool val)
{
    CameraEvent* event = new CameraEvent(CameraEvent::gp_busy);
    event->result = val;
    QApplication::postEvent(parent, event);
}

void CameraThread::sendError(const QString& msg)
{
    CameraEvent* event = new CameraEvent(CameraEvent::gp_errormsg);
    event->msg = msg;
    QApplication::postEvent(parent, event);
}


void CameraThread::sendInfo(const QString& msg)
{
    CameraEvent* event = new CameraEvent(CameraEvent::gp_infomsg);
    event->msg = msg;
    QApplication::postEvent(parent, event);
}

// -- Camera Controller ------------------------------------------------------

CameraController::CameraController(QWidget* parent, const QString& model,
                                   const QString& port,
                                   const QString& path)
    : QObject(parent)
{
    d = new CameraControllerPriv;	
    d->parent        = parent;
    d->close         = false;

    d->overwriteAll  = false;
    d->skipAll       = false;
    d->downloadTotal = 0;

    d->camera = new GPCamera(model, port, path);
    d->thread = new CameraThread(this);
    d->timer  = new QTimer();

    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotProcessNext()));

    d->timer->start(50, false);
}

CameraController::~CameraController()
{
    delete d->timer;
    
    d->camera->cancel();
    d->close = true;

    while (d->thread->running())
        d->thread->wait();
    delete d->thread;
    delete d->camera;
    delete d;
}

void CameraController::slotConnect()
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_connect;
    d->cmdQueue.enqueue(cmd);
}

void CameraController::listFolders()
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_listfolders;
    d->cmdQueue.enqueue(cmd);
}

void CameraController::listFiles(const QString& folder)
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_listfiles;
    cmd->map.insert("folder", QVariant(folder));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::getThumbnail(const QString& folder, const QString& file)
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_thumbnail;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::downloadPrep()
{
    d->overwriteAll  = false;
    d->skipAll       = false;
    d->downloadTotal = 0;
}

void CameraController::download(const QString& folder, const QString& file,
                                const QString& dest)
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_download;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    cmd->map.insert("dest", QVariant(dest));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::deleteFile(const QString& folder, const QString& file)
{
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_delete;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::slotCancel()
{
    d->cmdQueue.flush();   
    d->camera->cancel();
}

void CameraController::customEvent(QCustomEvent* e)
{
    CameraEvent* event = dynamic_cast<CameraEvent*>(e);
    if (!event)
    {
        return;
    }
    
    switch(event->type()-QEvent::User)
    {
    case (CameraEvent::gp_connected) :
    {
        emit signalConnected(event->result);
        break;
    }
    case (CameraEvent::gp_errormsg) :
    {
        emit signalErrorMsg(QDeepCopy<QString>(event->msg));
        break;
    }
    case (CameraEvent::gp_busy) :
    {
        if (event->result)
            emit signalBusy(true);
        break;
    }
    case (CameraEvent::gp_infomsg) :
    {
        emit signalInfoMsg(QDeepCopy<QString>(event->msg));
        break;
    }
    case (CameraEvent::gp_listedfolders) :
    {
        QStringList folderList =
            QDeepCopy<QStringList>(event->map["folders"].asStringList());
        emit signalFolderList(folderList);
        break;
    }
    case (CameraEvent::gp_listedfiles) :
    {
        QString    folder = QDeepCopy<QString>(event->map["folder"].asString());
        QByteArray ba     = QDeepCopy<QByteArray>(event->map["files"].asByteArray());
        QDataStream ds(ba, IO_ReadOnly);
        GPItemInfoList items;
        ds >> items;
        emit signalFileList(items);
        break;
    }
    case (CameraEvent::gp_thumbnailed) :
    {
        QString folder = QDeepCopy<QString>(event->map["folder"].asString());
        QString file   = QDeepCopy<QString>(event->map["file"].asString());
        QImage  thumb  = QDeepCopy<QImage>(event->map["thumbnail"].asImage());
        emit signalThumbnail(folder, file, thumb);
        break;
    }
    case (CameraEvent::gp_downloaded) :
    {
        QString folder = QDeepCopy<QString>(event->map["folder"].asString());
        QString file   = QDeepCopy<QString>(event->map["file"].asString());
        emit signalDownloaded(folder, file);
        break;
    }
    case (CameraEvent::gp_downloadFailed) :
    {
        QString folder = QDeepCopy<QString>(event->map["folder"].asString());
        QString file   = QDeepCopy<QString>(event->map["file"].asString());
 
        d->timer->stop();

        QString msg = i18n("Failed to download file %1.")
                      .arg(file);
        
        if (d->cmdQueue.isEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);
            if (result != KMessageBox::Continue)
                slotCancel();
        }

        d->timer->start(50);
        emit signalDownloaded(folder, file);
        break;
    }
    case (CameraEvent::gp_deleted) :
    {
        QString folder = QDeepCopy<QString>(event->map["folder"].asString());
        QString file   = QDeepCopy<QString>(event->map["file"].asString());
        emit signalDeleted(folder, file);
        break;
    }
    case (CameraEvent::gp_deleteFailed) :
    {
        QString folder = QDeepCopy<QString>(event->map["folder"].asString());
        QString file   = QDeepCopy<QString>(event->map["file"].asString());
 
        d->timer->stop();

        QString msg = i18n("Failed to delete file %1.")
                      .arg(file);
        
        if (d->cmdQueue.isEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);
            if (result != KMessageBox::Continue)
                slotCancel();
        }

        d->timer->start(50);
        break;
    }
    default:
        kdWarning() << k_funcinfo << "Unknown event" << endl;
    }
}

void CameraController::slotProcessNext()
{
    if (d->thread->running())
        return;

    if (d->cmdQueue.isEmpty())
    {
        emit signalBusy(false);
        return;
    }

    d->timer->stop();
    emit signalBusy(true);
    
    CameraCommand* cmd = d->cmdQueue.head();

    bool skip      = false;
    bool cancel    = false;
    bool overwrite = false;

    QString folder;
    QString file;
    QString dest;
    
    if (cmd->action == CameraCommand::gp_download)
    {
        folder = QDeepCopy<QString>(cmd->map["folder"].asString());
        file   = QDeepCopy<QString>(cmd->map["file"].asString());
        dest   = QDeepCopy<QString>(cmd->map["dest"].asString());

        if (!d->overwriteAll)
        {
            struct stat info;
            while (::stat(QFile::encodeName(dest), &info) == 0)
            {
                if (d->skipAll)
                {
                    skip = true;
                    break;
                }

                KIO::RenameDlg dlg(d->parent, i18n("Rename File"), file, dest,
                                   KIO::RenameDlg_Mode(KIO::M_MULTI |
                                                       KIO::M_OVERWRITE |
                                                       KIO::M_SKIP));
            
                int result = dlg.exec();
                dest       = dlg.newDestURL().path();

                switch (result)
                {
                case KIO::R_CANCEL:
                {
                    cancel = true;
                    break;
                }
                case KIO::R_SKIP:
                {
                    skip = true;
                    break;
                }
                case KIO::R_AUTO_SKIP:
                {
                    d->skipAll = true;
                    skip       = true;
                    break;
                }
                case KIO::R_OVERWRITE:
                {
                    overwrite       = true;
                    break;
                }
                case KIO::R_OVERWRITE_ALL:
                {
                    d->overwriteAll = true;
                    overwrite       = true;
                    break;
                }
                default:
                    break;
                }

                if (cancel || skip || overwrite)
                    break;
            }
        }

        cmd->map["dest"] = QVariant(QDeepCopy<QString>(dest));
        
    }

    if (cancel)
    {
        slotCancel();
        d->timer->start(50, false);
        return;
    }
    else if (skip)
    {
        d->cmdQueue.dequeue();
        emit signalInfoMsg(i18n("Skipped file %1")
                           .arg(file));
        emit signalSkipped(folder, file);        
        d->timer->start(50, false);
        return;
    }

    d->thread->start();
    d->timer->start(50, false);
}

#include "cameracontroller.moc"

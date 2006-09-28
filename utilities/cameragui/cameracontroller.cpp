/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-09-17
 * Description : digital camera controller
 * 
 * Copyright 2004-2005 by Renchi Raju
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes.

#include <typeinfo>
#include <cstdio>

// Qt includes.

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
#include <qregexp.h>

// KDE includes.

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/renamedlg.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes.

#include "thumbnailsize.h"
#include "imagewindow.h"
#include "gpcamera.h"
#include "umscamera.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "mtqueue.h"
#include "cameracontroller.h"
#include "cameracontroller.moc"

namespace Digikam
{

class CameraThread;

class CameraCommand
{
public:

    enum Action
    {
        gp_none = 0,
        gp_connect,
        gp_cancel,
        gp_cameraInformations,
        gp_listfolders,
        gp_listfiles,
        gp_download,
        gp_upload,
        gp_delete,
        gp_lock,
        gp_thumbnail,
        gp_exif,
        gp_open
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
        gp_downloadstarted,
        gp_downloaded,
        gp_downloadFailed,
        gp_opened,
        gp_uploaded,
        gp_uploadFailed,
        gp_deleted,
        gp_deleteFailed,
        gp_locked,
        gp_lockFailed,
        gp_thumbnailed,
        gp_exif,
        gp_cameraInformations,
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

    CameraControllerPriv()
    {
        parent = 0; 
        timer  = 0; 
        thread = 0; 
        camera = 0; 
    }
    
    bool                    close;
    bool                    overwriteAll;
    bool                    skipAll;
    bool                    canceled;
    
    int                     downloadTotal;
    
    QWidget                *parent;
    
    QTimer                 *timer;
    
    CameraThread           *thread;
    
    DKCamera               *camera;
    
    MTQueue<CameraCommand>  cmdQueue;
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
    
    CameraControllerPriv *d;
    
    QObject              *parent;
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
                sendInfo(i18n("Connecting to camera..."));
            
                bool result = d->camera->doConnect();
    
                CameraEvent* event = new CameraEvent(CameraEvent::gp_connected);
                event->result = result;
                QApplication::postEvent(parent, event);
    
                if (result)
                    sendInfo(i18n("Connection established"));
                else
                    sendInfo(i18n("Connection failed"));
    
                break;
            }
            case(CameraCommand::gp_cameraInformations):
            {
                sendInfo(i18n("Getting camera information..."));
            
                QString summary, manual, about;

                d->camera->cameraSummary(summary);
                d->camera->cameraManual(manual);
                d->camera->cameraAbout(about);
            
                CameraEvent* event = new CameraEvent(CameraEvent::gp_cameraInformations);
                event->map.insert("summary", QVariant(summary));
                event->map.insert("manual", QVariant(manual));
                event->map.insert("about", QVariant(about));
                QApplication::postEvent(parent, event);
                break;
            }            
            case(CameraCommand::gp_listfolders):
            {
                sendInfo(i18n("Listing folders..."));
    
                QStringList folderList;
                folderList.append(d->camera->path());
                d->camera->getAllFolders(d->camera->path(), folderList);
    
                /* TODO: ugly hack since qt <= 3.1.2 does not define
                QStringList with QDeepCopy as a friend. */
                QValueList<QString> flist(folderList);
                
                CameraEvent* event = new CameraEvent(CameraEvent::gp_listedfolders);
                event->map.insert("folders", QVariant(flist));
                QApplication::postEvent(parent, event);
            
                sendInfo(i18n("Listing folders is complete..."));
    
                break;
            }
            case(CameraCommand::gp_listfiles):
            {
                QString folder = cmd->map["folder"].asString();
                
                sendInfo(i18n("Listing files in %1...").arg(folder));
    
                GPItemInfoList itemsList;
                // setting getImageDimensions to false is a huge speedup for UMSCamera
                if (!d->camera->getItemsInfoList(folder, itemsList, false))
                {
                    sendError(i18n("Failed to list files in %1").arg(folder));
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
    
                sendInfo(i18n("Listing files in %1 is complete").arg(folder));
                
                break;
            }
            case(CameraCommand::gp_thumbnail):
            {
                QString folder = cmd->map["folder"].asString();
                QString file   = cmd->map["file"].asString();
    
                sendInfo(i18n("Getting thumbnail for %1/%2...").arg(folder).arg(file));
    
                QImage thumbnail;
                d->camera->getThumbnail(folder, file, thumbnail);
    
                if (!thumbnail.isNull())
                {
                    thumbnail = thumbnail.smoothScale(ThumbnailSize::Huge, ThumbnailSize::Huge, QImage::ScaleMin);
            
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_thumbnailed);
                    event->map.insert("folder", QVariant(folder));
                    event->map.insert("file", QVariant(file));
                    event->map.insert("thumbnail", QVariant(thumbnail));
                    QApplication::postEvent(parent, event);
                }
    
                break;
            }
            case(CameraCommand::gp_exif):
            {
                QString folder = cmd->map["folder"].asString();
                QString file   = cmd->map["file"].asString();
    
                sendInfo(i18n("Getting EXIF information for %1/%2...").arg(folder).arg(file));
    
                char* edata = 0;
                int   esize = 0;
                d->camera->getExif(folder, file, &edata, esize);
    
                if (edata || esize)
                {
                    QByteArray  ba;
                    QDataStream ds(ba, IO_WriteOnly);
                    ds.writeRawBytes(edata, esize);
                    delete [] edata;
            
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_exif);
                    event->map.insert("folder", QVariant(folder));
                    event->map.insert("file", QVariant(file));
                    event->map.insert("exifSize", QVariant(esize));
                    event->map.insert("exifData", QVariant(ba));
                    QApplication::postEvent(parent, event);
                }
                break;
            }
            case(CameraCommand::gp_download):
            {
                QString   folder            = cmd->map["folder"].asString();
                QString   file              = cmd->map["file"].asString();
                QString   dest              = cmd->map["dest"].asString();
                bool      autoRotate        = cmd->map["autoRotate"].asBool();
                bool      fixDateTime       = cmd->map["fixDateTime"].asBool();
                QDateTime newDateTime       = cmd->map["newDateTime"].asDateTime();
                bool      setPhotographerId = cmd->map["setPhotographerId"].asBool();
                QString   author            = cmd->map["author"].asString();
                QString   authorTitle       = cmd->map["authorTitle"].asString();
                bool      setCredits        = cmd->map["setCredits"].asBool();
                QString   credit            = cmd->map["credit"].asString();
                QString   source            = cmd->map["source"].asString();
                QString   copyright         = cmd->map["copyright"].asString();
                bool      convertJpeg       = cmd->map["convertJpeg"].asBool();
                QString   losslessFormat    = cmd->map["losslessFormat"].asString();
                sendInfo(i18n("Downloading file %1...").arg(file));
    
                // download to a temp file

                CameraEvent* event = new CameraEvent(CameraEvent::gp_downloadstarted);
                event->map.insert("folder", QVariant(folder));
                event->map.insert("file", QVariant(file));
                event->map.insert("dest", QVariant(dest));
                QApplication::postEvent(parent, event);

                KURL tempURL(dest);
                tempURL = tempURL.upURL();
                tempURL.addPath( QString(".digikam-camera-tmp1-%1").arg(getpid()));
    
                bool result = d->camera->downloadItem(folder, file, tempURL.path());
    
                if (result)
                {
                    if (autoRotate)
                    {
                        kdDebug() << "Exif autorotate: " << file << " using (" << tempURL.path() << ")" << endl;
                        sendInfo(i18n("EXIF rotating file %1...").arg(file));
                        exifRotate(tempURL.path(), file);
                    }
    
                    if (fixDateTime || setPhotographerId || setCredits)
                    {
                        sendInfo(i18n("Setting Metadata tags to file %1...").arg(file));
                        DMetadata metadata(tempURL.path());
                        
                        if (fixDateTime)
                            metadata.setImageDateTime(newDateTime, true);
                        
                        if (setPhotographerId)
                            metadata.setImagePhotographerId(author, authorTitle);
    
                        if (setCredits)
                            metadata.setImageCredits(credit, source, copyright);
                                                                    
                        metadata.applyChanges();
                    }
                    
                    // Convert Jpeg file to lossless format if necessary, 
                    // and move converted image to destination.

                    if (convertJpeg && isJpegImage(tempURL.path()))
                    {
                        sendInfo(i18n("Converting %1 to lossless file format...").arg(file));

                        KURL tempURL2(dest);
                        tempURL2 = tempURL2.upURL();
                        tempURL2.addPath( QString(".digikam-camera-tmp2-%1").arg(getpid()));

                        if (!jpegConvert(tempURL.path(), tempURL2.path(), file, losslessFormat))
                        {
                            // convert failed. delete the temp file
                            unlink(QFile::encodeName(tempURL2.path()));
                            result = false;
                        }
                        else
                        {
                            // move the file to the destination file
                            if (rename(QFile::encodeName(tempURL2.path()), QFile::encodeName(dest)) != 0)
                            {
                                // rename failed. delete the temp file
                                unlink(QFile::encodeName(tempURL2.path()));
                                result = false;
                            }
                        }

                        unlink(QFile::encodeName(tempURL.path()));
                    }
                    else
                    {
                        // move the file to the destination file
                        if (rename(QFile::encodeName(tempURL.path()), QFile::encodeName(dest)) != 0)
                        {
                            // rename failed. delete the temp file
                            unlink(QFile::encodeName(tempURL.path()));
                            result = false;
                        }
                    }
                }
    
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
            case(CameraCommand::gp_open):
            {
                QString folder = cmd->map["folder"].asString();
                QString file   = cmd->map["file"].asString();
                QString dest   = cmd->map["dest"].asString();
    
                sendInfo(i18n("Retrieving file %1 from camera...").arg(file));
    
                bool result = d->camera->downloadItem(folder, file, dest);
    
                if (result)
                {
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_opened);
                    event->map.insert("folder", QVariant(folder));
                    event->map.insert("file", QVariant(file));
                    event->map.insert("dest", QVariant(dest));
                    QApplication::postEvent(parent, event);
                }
                else
                {
                    sendError(i18n("Failed to retrieve file %1 from camera").arg(file));
                }                
                break;
            }
            case(CameraCommand::gp_upload):
            {                
                QString folder = cmd->map["destFolder"].asString();
    
                // We will using the same source file name to create the dest file 
                // name in camera.
                QString file   = cmd->map["destFile"].asString();
                
                // The source file path to download in camera.
                QString src    = cmd->map["srcFilePath"].asString();
    
                sendInfo(i18n("Uploading file %1 to camera...").arg(file));
    
                GPItemInfo itemsInfo;

                bool result = d->camera->uploadItem(folder, file, src, itemsInfo);
    
                if (result)
                {
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_uploaded);
                    QByteArray  ba;
                    QDataStream ds(ba, IO_WriteOnly);
                    ds << itemsInfo;                    
                    event->map.insert("info", QVariant(ba));

                    QApplication::postEvent(parent, event);
                }
                else
                {
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_uploadFailed);
                    event->map.insert("folder", QVariant(folder));
                    event->map.insert("file", QVariant(file));
                    event->map.insert("src", QVariant(src));
                    QApplication::postEvent(parent, event);
                }                
                break;
            }
            case(CameraCommand::gp_delete):
            {
                QString folder = cmd->map["folder"].asString();
                QString file   = cmd->map["file"].asString();
    
                sendInfo(i18n("Deleting file %1...").arg(file));
    
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
            case(CameraCommand::gp_lock):
            {
                QString folder = cmd->map["folder"].asString();
                QString file   = cmd->map["file"].asString();
                bool    lock   = cmd->map["lock"].asBool();
    
                sendInfo(i18n("Toggle lock file %1...").arg(file));
    
                bool result = d->camera->setLockItem(folder, file, lock);
    
                if (result)
                {
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_locked);
                    event->map.insert("folder", QVariant(folder));
                    event->map.insert("file", QVariant(file));
                    QApplication::postEvent(parent, event);
                }
                else
                {
                    CameraEvent* event = new CameraEvent(CameraEvent::gp_lockFailed);
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


//-- Camera Controller ------------------------------------------------------


CameraController::CameraController(QWidget* parent, const QString& title, const QString& model,
                                   const QString& port, const QString& path)
                : QObject(parent)
{
    d = new CameraControllerPriv;	
    d->parent        = parent;
    d->canceled      = false;
    d->close         = false;
    d->overwriteAll  = false;
    d->skipAll       = false;
    d->downloadTotal = 0;
    d->camera        = 0;

    // URL parsing (c) Stephan Kulow
    if (path.startsWith("camera:/"))
    {
        KURL url(path);
        kdDebug() << "path " << path << " " << url <<  " " << url.host() << endl;
        QString xport = url.host();
        if (xport.startsWith("usb:"))
        {
            kdDebug() << "xport " << xport << endl;
            QRegExp x = QRegExp("(usb:[0-9,]*)");

            if (x.search(xport) != -1) 
            {
                QString usbport = x.cap(1);
                kdDebug() << "USB " << xport << " " << usbport << endl;
                // if ((xport == usbport) || ((count == 1) && (xport == "usb:"))) {
                //   model = xmodel;
                d->camera = new GPCamera(title, url.user(), "usb:", "/");
                // }
            }
        }
    }

    if (!d->camera)
    {
        if (model.lower() == "directory browse")
            d->camera = new UMSCamera(title, model, port, path);
        else
            d->camera = new GPCamera(title, model, port, path);
    }

    d->thread = new CameraThread(this);
    d->timer  = new QTimer();

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotProcessNext()));

    d->timer->start(50, false);
}

CameraController::~CameraController()
{
    if (d->timer->isActive()) 
    {
        d->timer->stop();
        delete d->timer;
    }
    
    d->camera->cancel();
    d->canceled = true;
    d->close    = true;

    while (d->thread->running())
        d->thread->wait();
        
    delete d->thread;
    delete d->camera;
    delete d;
}

QString CameraController::getCameraPath()
{
    return d->camera->path();
}

QString CameraController::getCameraTitle()
{
    return d->camera->title();
}

void CameraController::slotConnect()
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_connect;
    d->cmdQueue.enqueue(cmd);
}

void CameraController::listFolders()
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_listfolders;
    d->cmdQueue.enqueue(cmd);
}

void CameraController::listFiles(const QString& folder)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_listfiles;
    cmd->map.insert("folder", QVariant(folder));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::getThumbnail(const QString& folder, const QString& file)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_thumbnail;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::getExif(const QString& folder, const QString& file)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_exif;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::getCameraInformations()
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_cameraInformations;
    d->cmdQueue.enqueue(cmd);
}

void CameraController::upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_upload;
    cmd->map.insert("srcFilePath", QVariant(srcFileInfo.filePath()));
    cmd->map.insert("destFile", QVariant(destFile));
    cmd->map.insert("destFolder", QVariant(destFolder));
    d->cmdQueue.enqueue(cmd);
    kdDebug() << "Uploading '" << srcFileInfo.filePath() << "' into camera : '" << destFolder << 
                 "' (" << destFile << ")" << endl;
}

void CameraController::downloadPrep()
{
    d->overwriteAll  = false;
    d->skipAll       = false;
    d->downloadTotal = 0;
}

void CameraController::download(DownloadSettingsContainer downloadSettings)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_download;
    cmd->map.insert("folder", QVariant(downloadSettings.folder));
    cmd->map.insert("file", QVariant(downloadSettings.file));
    cmd->map.insert("dest", QVariant(downloadSettings.dest));
    cmd->map.insert("autoRotate", QVariant(downloadSettings.autoRotate, 0));
    cmd->map.insert("fixDateTime", QVariant(downloadSettings.fixDateTime, 0));
    cmd->map.insert("newDateTime", QVariant(downloadSettings.newDateTime));
    cmd->map.insert("setPhotographerId", QVariant(downloadSettings.setPhotographerId, 0));
    cmd->map.insert("author", QVariant(downloadSettings.author));
    cmd->map.insert("authorTitle", QVariant(downloadSettings.authorTitle));
    cmd->map.insert("setCredits", QVariant(downloadSettings.setCredits, 0));
    cmd->map.insert("credit", QVariant(downloadSettings.credit));
    cmd->map.insert("source", QVariant(downloadSettings.source));
    cmd->map.insert("copyright", QVariant(downloadSettings.copyright));
    cmd->map.insert("convertJpeg", QVariant(downloadSettings.convertJpeg, 0));
    cmd->map.insert("losslessFormat", QVariant(downloadSettings.losslessFormat));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::deleteFile(const QString& folder, const QString& file)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_delete;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::lockFile(const QString& folder, const QString& file, bool lock)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_lock;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    cmd->map.insert("lock", QVariant(lock, 0));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::openFile(const QString& folder, const QString& file)
{
    d->canceled = false;
    CameraCommand *cmd = new CameraCommand;
    cmd->action = CameraCommand::gp_open;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file", QVariant(file));
    cmd->map.insert("dest", QVariant(locateLocal("tmp", file)));
    d->cmdQueue.enqueue(cmd);
}

void CameraController::slotCancel()
{
    d->canceled = true;
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
        case (CameraEvent::gp_cameraInformations) :
        {
            QString summary = QDeepCopy<QString>(event->map["summary"].asString());
            QString manual  = QDeepCopy<QString>(event->map["manual"].asString());
            QString about   = QDeepCopy<QString>(event->map["about"].asString());
            emit signalCameraInformations(summary, manual, about);
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
            if (!d->canceled)
                emit signalInfoMsg(QDeepCopy<QString>(event->msg));
            break;
        }
        case (CameraEvent::gp_listedfolders) :
        {
            /* TODO: ugly hack since qt <= 3.1.2 does not define
            QStringList with QDeepCopy as a friend. */
            QValueList<QVariant> flist = QDeepCopy< QValueList<QVariant> >(event->map["folders"].toList());
    
            QStringList folderList;
            QValueList<QVariant>::Iterator it;
            for (it = flist.begin(); it != flist.end(); ++it )
            {
                folderList.append(QDeepCopy<QString>((*it).asString()));
            }
            
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
        case (CameraEvent::gp_exif) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            QByteArray ba  = QDeepCopy<QByteArray>(event->map["exifData"].asByteArray());
            emit signalExifData(ba);
            break;
        }
        case (CameraEvent::gp_downloadstarted) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            emit signalDownloaded(folder, file, GPItemInfo::DownloadStarted);
            break;
        }
        case (CameraEvent::gp_downloaded) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            emit signalDownloaded(folder, file, GPItemInfo::DownloadedYes);
            break;
        }
        case (CameraEvent::gp_downloadFailed) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
    
            d->timer->stop();
    
            QString msg = i18n("Failed to download file \"%1\".").arg(file);
            
            if (!d->canceled)
            {
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
            }
    
            d->timer->start(50);
            emit signalDownloaded(folder, file, GPItemInfo::DownloadFailed);
            break;
        }
        case (CameraEvent::gp_uploaded) :
        {
            QByteArray ba  = QDeepCopy<QByteArray>(event->map["info"].asByteArray());
            QDataStream ds(ba, IO_ReadOnly);
            GPItemInfo itemInfo;
            ds >> itemInfo;

            emit signalUploaded(itemInfo);
            break;
        }
        case (CameraEvent::gp_uploadFailed) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            QString src    = QDeepCopy<QString>(event->map["src"].asString());

            d->timer->stop();
    
            QString msg = i18n("Failed to upload file \"%1\".").arg(file);
            
            if (!d->canceled)
            {
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
            }
    
            d->timer->start(50);
            break;
        }
        case (CameraEvent::gp_deleted) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            emit signalDeleted(folder, file, true);
            break;
        }
        case (CameraEvent::gp_deleteFailed) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
    
            d->timer->stop();
            emit signalDeleted(folder, file, false);

            QString msg = i18n("Failed to delete file \"%1\".").arg(file);
            
            if (!d->canceled)
            {
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
            }
    
            d->timer->start(50);
            break;
        }
        case (CameraEvent::gp_locked) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
            emit signalLocked(folder, file, true);
            break;
        }
        case (CameraEvent::gp_lockFailed) :
        {
            QString folder = QDeepCopy<QString>(event->map["folder"].asString());
            QString file   = QDeepCopy<QString>(event->map["file"].asString());
    
            d->timer->stop();
            emit signalLocked(folder, file, false);

            QString msg = i18n("Failed to toggle lock file \"%1\".").arg(file);
            
            if (!d->canceled)
            {
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
            }
    
            d->timer->start(50);
            break;
        }
        case (CameraEvent::gp_opened) :
        {
            QString file = QDeepCopy<QString>(event->map["file"].asString());
            QString dest = QDeepCopy<QString>(event->map["dest"].asString());
    
            KURL url(dest);
            KURL::List urlList;
            urlList << url;
    
            ImageWindow *im = ImageWindow::imagewindow();
            im->loadURL(urlList, url, i18n("Camera \"%1\"").arg(d->camera->model()), false);
    
            if (im->isHidden())
                im->show();
            else
                im->raise();
                
            im->setFocus();
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

    if ((cmd->action == CameraCommand::gp_exif) &&
        (typeid(*(d->camera)) == typeid(UMSCamera)))
    {
        folder = QDeepCopy<QString>(cmd->map["folder"].asString());
        file   = QDeepCopy<QString>(cmd->map["file"].asString());

        emit signalExifFromFile(folder, file);

        d->cmdQueue.dequeue();
        d->timer->start(50, false);
        return;
    }
      
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

                // FIXME : see B.K.O #126427: with Gphoto camera, the camera folder is not 
                // mounted in local and camera picture cannot be display like a preview in dialog.

                KIO::RenameDlg dlg(d->parent, i18n("Rename File"), folder + QString("/") + file, dest,
                                   KIO::RenameDlg_Mode(KIO::M_MULTI | KIO::M_OVERWRITE | KIO::M_SKIP));
            
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
                        overwrite = true;
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
        emit signalInfoMsg(i18n("Skipped file %1").arg(file));
        emit signalSkipped(folder, file);        
        d->timer->start(50, false);
        return;
    }

    d->thread->start();
    d->timer->start(50, false);
}

}  // namespace Digikam


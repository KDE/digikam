/* ============================================================
 * File  : cameracontroller.h
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

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <kfileitem.h>
#include <kio/global.h>
#include <qobject.h>

namespace KIO
{
class Job;
class Slave;
}

class QWidget;
class QString;
class QPixmap;
class CameraControllerPriv;

class CameraController : public QObject
{
    Q_OBJECT

public:

    enum State {
        ST_NONE = 0,
        ST_CONNECT,
        ST_LIST,
        ST_THUMBNAIL,
        ST_DOWNLOAD,
        ST_DELETE,
        ST_EXIF,
        ST_INFO,
        ST_OPEN
    };
    
    CameraController(QWidget* parent, const QString& model,
                     const QString& port, const QString& path);
    ~CameraController();

    void downloadAll(const QString& destFolder);
    void downloadSel(const KFileItemList& items, const QString& destFolder);
    void deleteAll();
    void deleteSel(const KFileItemList& items);

    void getExif(const KFileItem* item);
    
private:

    CameraControllerPriv* d;

signals:

    void signalFatal(const QString& msg);
    void signalBusy(bool val);
    void signalInfoMessage(const QString& msg);
    void signalInfoPercent(int percent);

    void signalNewItems(const KFileItemList& itemList);
    void signalClear();
    void signalThumbnail(const KFileItem* item, const QPixmap& pix);

public slots:

    void slotCameraInfo();
    void slotCancel();
    
private slots:

    void slotSlaveConnected(KIO::Slave*);
    void slotSlaveConnectFailed();
    void slotSlaveError(KIO::Slave *slave, int error,
                        const QString &errorMsg);

    void slotResult(KIO::Job *job);
    void slotInfoMessage (KIO::Job *job, const QString &msg);
    void slotPercent (KIO::Job *job, unsigned long percent);

    void slotList();
    void slotEntries(KIO::Job*, const KIO::UDSEntryList&);
    void slotThumbnails();

    void slotExifData(KIO::Job *job, const QByteArray &data);
    void slotInfoData(KIO::Job *job, const QByteArray &data);
};

#endif /* CAMERACONTROLLER_H */

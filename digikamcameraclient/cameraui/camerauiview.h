/* ============================================================
 * File  : camerauiview.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef CAMERAUIVIEW_H
#define CAMERAUIVIEW_H

#include <qwidget.h>
#include <qdict.h>
#include <qstring.h>

#include <kfileitem.h>

#include "gpfileiteminfo.h"
#include "thumbnailsize.h"
#include "cameratype.h"

class QImage;
class QPushButton;
class KComboBox;
class KDirLister;

class GPEventFilter;
class GPController;
class GPFileItemContainer;
class CameraFolderView;
class CameraIconView;
class CameraIconItem;
class CameraFolderItem;
class CameraUI;
class CameraSettings;

namespace KIO
{
class Job;
}

class CameraUIView : public QWidget
{
    Q_OBJECT

public:

    CameraUIView(QWidget* parent, const CameraType& ctype);
    ~CameraUIView();

    void applySettings(CameraSettings *cameraSettings);
    void setLibraryPath(const QString& libraryPath);
    void setCurrentAlbum(const QString& album);
    
    void cameraInitialized(bool val);
    void cameraSubFolder(const QString& folder,
                         const QString& subFolder);
    void cameraNewItems(const QString& folder,
                        const GPFileItemInfoList& infoList);
    void cameraNewItems(const GPFileItemInfoList& infoList);
    void cameraNewThumbnail(const QString& folder,
                            const QString& itemName,
                            const QImage&  thumbnail);
    void cameraDownloadedItem(const QString& folder,
                              const QString& itemName);
    void cameraDeletedItem(const QString& folder,
                          const QString& itemName);
    void cameraDeletedAllItems();
    void cameraOpenedItem(const QString& fileName);
    void cameraOpenedItem(const QString& fileName,
                          const QString& serviceName);
    void cameraExifInfo(const QString& folder,
                        const QString& itemName,
                        char* data, int size);
    void cameraErrorMsg(const QString& msg);
    
private:

    void setupConnections();
    bool cameraReadyForUpload(QString& reason);

public slots:

    void slotCameraConnectToggle();
    void slotCameraDownloadSelected();
    void slotCameraDownloadAll();
    void slotCameraDeleteSelected();
    void slotCameraDeleteAll();
    void slotCameraUpload();
    void slotCameraOpenItem(const QString& folder,
                            const QString& itemName);
    void slotCameraInformation();
    void slotCameraCancel();
    
    void slotThumbSizePlus();
    void slotThumbSizeMinus();
    void slotShowFoldersToggle();

    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSelectNew();


private slots:

    // -- Slots handling folderview signals --------------------

    void slotFolderSelected(CameraFolderItem *item);

    // -- Slots handling iconview signals --------------------

    void slotCameraOpenItem(const QString& folder,
                            const QString& itemName,
                            const QString& serviceName);
    void slotCameraExifInformation(const QString& folder,
                                   const QString& itemName);

    // -- Slots handling dirlister signals -------------------

    void slotNewAlbums(const KFileItemList& fileItemList);
    void slotDeleteAlbum(KFileItem* fileItem);
    void slotClearAlbums();
    
    // -- Slot handling new album button signal -------------

    void slotCreateNewAlbum();
    void slotOnAlbumCreate(KIO::Job* job);
    
signals:

    void signalStatusMsg(const QString&);
    void signalProgressVal(int);
    void signalBusy(bool);
    
private:

    void downloadOneItem(const QString& item, const QString& folder,
                         const QString& downloadDir, bool& proceedFurther,
                         bool& overwriteAll);
    
    GPEventFilter       *efilter_;
    GPController        *controller_;
    GPFileItemContainer *container_;
    CameraFolderView    *folderView_;
    CameraIconView      *iconView_;
    KDirLister          *dirLister_;
    
    ThumbnailSize     thumbSize_;
    CameraUI         *parent_;
    QString           cameraPath_;
    bool              cameraConnected_;
    KComboBox        *downloadAlbumBox_;
    QPushButton      *newAlbumButton_;
    CameraType        cameraType_;

    QString           libraryPath_;
    QString           currentAlbum_;
    bool              creatingNewAlbum;
};

#endif /* CAMERAUIVIEW_H */

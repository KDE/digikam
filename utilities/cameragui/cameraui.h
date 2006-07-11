/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-16
 * Description : Camera interface dialog
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

#ifndef CAMERAUI_H
#define CAMERAUI_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// KDE includes.

#include <kdialogbase.h>
#include <kurl.h>

// Local includes.

#include "gpiteminfo.h"

namespace Digikam
{

class CameraIconViewItem;
class CameraUIPriv;

class CameraUI : public KDialogBase
{
    Q_OBJECT

public:

    CameraUI(QWidget* parent, const QString& cameraTitle,
             const QString& model, const QString& port,
             const QString& path);
    ~CameraUI();

    bool isBusy() const;
    bool isClosed() const;
    
signals:
    
    void signalLastDestination(const KURL&);
    void signalAlbumSettingsChanged();

protected:

    void closeEvent(QCloseEvent* e);
    
private:

    void readSettings();
    void saveSettings();
    bool dialogClosed();
    bool createAutoAlbum(const KURL& parentURL, const QString& name,
                         const QDate& date, QString& errMsg);
    void addFileExtension(const QString& ext);
    void finishDialog();

private slots:

    void slotClose();
    void slotCancelButton();
    void slotProcessURL(const QString& url);

    void slotConnected(bool val);
    void slotBusy(bool val);
    void slotErrorMsg(const QString& msg);
    void slotFolderList(const QStringList& folderList);
    void slotFileList(const GPItemInfoList& fileList);
    void slotThumbnail(const QString& folder, const QString& file,
                       const QImage& thumbnail);

    void slotDownloadSelected();
    void slotDownloadAll();
    void slotDownload(bool onlySelected);
    void slotDeleteSelected();
    void slotDeleteAll();

    void slotFileView(CameraIconViewItem* item);

    void slotDownloaded(const QString&, const QString&);
    void slotSkipped(const QString&, const QString&);
    void slotDeleted(const QString&, const QString&);
    
    void slotItemsSelected(CameraIconViewItem* item, bool selected);
    
    void slotExifFromFile(const QString& folder, const QString& file);
    void slotExifFromData(const QByteArray& exifData);

    void slotFirstItem(void);
    void slotPrevItem(void);    
    void slotNextItem(void);
    void slotLastItem(void);

private:
    
    CameraUIPriv* d;
};

}  // namespace Digikam

#endif /* CAMERAUI_H */

/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-16
 * Description : 
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

#include <qdialog.h>
#include <qstring.h>

// Local includes.

#include "gpiteminfo.h"

class QPushButton;
class QToolButton;
class QCheckBox;
class QLabel;
class QImage;
class QVButtonGroup;
class QVBox;
class QLineEdit;
class QPopupMenu;
class QProgressBar;
class QDate;
class QSplitter;

class KURL;

namespace Digikam
{

class CameraIconView;
class CameraIconViewItem;
class CameraController;
class RenameCustomizer;
class AnimWidget;
class ImagePropertiesSideBarCamGui;

class CameraUI : public QDialog
{
    Q_OBJECT

public:

    CameraUI(QWidget* parent, const QString& title,
             const QString& model, const QString& port,
             const QString& path);
    ~CameraUI();

    bool isBusy() const;
    
protected:

    void closeEvent(QCloseEvent* e);
    
private:

    void readSettings();
    void saveSettings();
    bool createAutoAlbum(const KURL& parentURL,
                         const QString& name,
                         const QDate& date,
                         QString& errMsg);
    void addFileExtension(const QString& ext);

private slots:

    void slotHelp();
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
    
    void slotToggleAdvanced();

    void slotExifFromFile(const QString& folder, const QString& file);
    void slotExifFromData(const QByteArray& exifData);

    void slotFirstItem(void);
    void slotPrevItem(void);    
    void slotNextItem(void);
    void slotLastItem(void);

signals:
    
    void signalLastDestination(const KURL&);
    void signalAlbumSettingsChanged();

private:
    
    bool              m_showAdvanced;
    bool              m_busy;

    QStringList       m_foldersToScan;

    QPushButton*      m_helpBtn;
    QPushButton*      m_closeBtn;
    QPushButton*      m_downloadBtn;
    QPushButton*      m_deleteBtn;
    QPushButton*      m_advBtn;

    QPopupMenu*       m_downloadMenu;
    QPopupMenu*       m_deleteMenu;

    QToolButton*      m_cancelBtn;

    QVBox*            m_advBox;

    QCheckBox*        m_autoRotateCheck;
    QCheckBox*        m_autoAlbumCheck;

    QLabel*           m_status;

    QProgressBar*     m_progress;

    QSplitter        *m_splitter;

    KURL              m_lastDestURL;

    CameraController* m_controller;

    CameraIconView*   m_view;

    RenameCustomizer* m_renameCustomizer;

    AnimWidget*       m_anim;

    ImagePropertiesSideBarCamGui *m_rightSidebar;
};

}  // namespace Digikam

#endif /* CAMERAUI_H */

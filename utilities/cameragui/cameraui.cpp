/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-16
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

// Qt includes.

#include <qvgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpopupmenu.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurl.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <khelpmenu.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "gpiteminfodlg.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameraui.h"
#include "scanlib.h"

namespace Digikam
{

CameraUI::CameraUI(QWidget* parent, const QString& title,
                   const QString& model, const QString& port,
                   const QString& path)
    : QDialog(parent, 0, false, WDestructiveClose)    
{
    // -- setup view -----------------------------------------

    QVBoxLayout* mainLayout = new QVBoxLayout(this, 5, 5);

    QGroupBox* viewBox = new QGroupBox(title, this);
    viewBox->setColumnLayout(0, Qt::Vertical);

    QGridLayout* viewBoxLayout =
        new QGridLayout(viewBox->layout(), 2, 4, 5);
    viewBoxLayout->setColStretch( 0, 0 );
    viewBoxLayout->setColStretch( 1, 3 );
    viewBoxLayout->setColStretch( 2, 1 );
    viewBoxLayout->setColStretch( 3, 0 );

    m_view = new CameraIconView(this, viewBox);
    viewBoxLayout->addMultiCellWidget(m_view, 0, 0, 0, 3);

    m_cancelBtn = new QToolButton(viewBox);
    QIconSet iconSet = kapp->iconLoader()->loadIconSet("stop",
                                                       KIcon::Toolbar,
                                                       22);
    m_cancelBtn->setText(i18n("Cancel"));
    m_cancelBtn->setIconSet(iconSet);
    m_cancelBtn->setEnabled(false);
    viewBoxLayout->addWidget(m_cancelBtn, 1, 0);
    
    m_status = new QLabel(viewBox);
    viewBoxLayout->addWidget(m_status, 1, 1);
    m_progress = new QProgressBar(viewBox);
    viewBoxLayout->addWidget(m_progress, 1, 2);
    m_progress->hide();
    m_anim = new AnimWidget(viewBox);
    viewBoxLayout->addWidget(m_anim, 1, 3);
    
    mainLayout->addWidget(viewBox);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    QPushButton* helpBtn;
    QPushButton* selectBtn;
    
    m_advBtn      = new QPushButton(i18n("&Advanced %1").arg(">>"), this);
    helpBtn       = new QPushButton(i18n("&Help"), this);
    selectBtn     = new QPushButton(i18n("&Select"), this);
    m_downloadBtn = new QPushButton(i18n("&Download"), this);
    m_deleteBtn   = new QPushButton(i18n("D&elete"), this);
    m_closeBtn    = new QPushButton(i18n("&Close"), this);
    
    btnLayout->addWidget(m_advBtn);
    btnLayout->addItem(new QSpacerItem(10,10,QSizePolicy::Expanding,
                                       QSizePolicy::Fixed));
    btnLayout->addWidget(helpBtn);
    btnLayout->addWidget(selectBtn);
    btnLayout->addWidget(m_downloadBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
    
    m_advBox = new QVBox(this);
    m_advBox->setSpacing(5);
    m_showAdvanced = false;
    m_advBox->hide();

    m_renameCustomizer = new RenameCustomizer(m_advBox);
    m_view->setRenameCustomizer(m_renameCustomizer);

    QVGroupBox* exifBox = new QVGroupBox(i18n("Use Camera-provided Information (EXIF)"),
                                         m_advBox);
    m_autoRotateCheck = new QCheckBox(i18n("Automatically rotate/flip using "
                                           "camera-provided information (EXIF)"),
                                      exifBox);
    m_autoAlbumCheck = new QCheckBox(i18n("Download photos into automatically "
                                          "created date-based sub-albums of "
                                          "destination album"),
                                      exifBox);
    
    mainLayout->addWidget(m_advBox);

    // About popupmenu button using a slot for calling the camera interface
    // anchor in Digikam handbook.
    
    KHelpMenu* helpMenu = new KHelpMenu(this, KApplication::kApplication()->aboutData(),
                                        false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Digikam Handbook"),
                                 this, SLOT(slotHelp()), 0, -1, 0);
    helpBtn->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------------------
    
    QPopupMenu* selectMenu = new QPopupMenu(this);
    selectMenu->insertItem(i18n("Select &All"),
                           m_view, SLOT(slotSelectAll()));
    selectMenu->insertItem(i18n("Select N&one"),
                           m_view, SLOT(slotSelectNone()));
    selectMenu->insertItem(i18n("&Invert Selection"),
                           m_view, SLOT(slotSelectInvert()));
    selectMenu->insertSeparator();
    selectMenu->insertItem(i18n("Select &New Items"),
                           m_view, SLOT(slotSelectNew()));
    selectBtn->setPopup(selectMenu);    

    // -------------------------------------------------------------------------
        
    m_downloadMenu = new QPopupMenu(this);
    m_downloadMenu->insertItem(i18n("Download Selected"),
                               this, SLOT(slotDownloadSelected()),
                               0, 0);
    m_downloadMenu->insertItem(i18n("Download All"),
                               this, SLOT(slotDownloadAll()),
                               0, 1);
    m_downloadMenu->setItemEnabled(0, false);
    m_downloadBtn->setPopup(m_downloadMenu);

    // -------------------------------------------------------------------------
    
    m_deleteMenu = new QPopupMenu(this);
    m_deleteMenu->insertItem(i18n("Delete Selected"),
                             this, SLOT(slotDeleteSelected()),
                             0, 0);
    m_deleteMenu->insertItem(i18n("Delete All"),
                             this, SLOT(slotDeleteAll()),
                             0, 1);
    m_deleteMenu->setItemEnabled(0, false);
    m_deleteBtn->setPopup(m_deleteMenu);

    // -------------------------------------------------------------------------
    
    connect(m_closeBtn, SIGNAL(clicked()),
            SLOT(close()));
    connect(m_advBtn, SIGNAL(clicked()),
            SLOT(slotToggleAdvanced()));

    connect(m_view, SIGNAL(signalSelected(bool)),
            SLOT(slotItemsSelected(bool)));
    connect(m_view, SIGNAL(signalFileView(CameraIconViewItem*)),
            SLOT(slotFileView(CameraIconViewItem*)));
    connect(m_view, SIGNAL(signalFileProperties(CameraIconViewItem*)),
            SLOT(slotFileProps(CameraIconViewItem*)));
    connect(m_view, SIGNAL(signalFileExif(CameraIconViewItem*)),
            SLOT(slotFileExif(CameraIconViewItem*)));
    connect(m_view, SIGNAL(signalDownload()),
            SLOT(slotDownloadSelected()));
    connect(m_view, SIGNAL(signalDelete()),
            SLOT(slotDeleteSelected()));

    // -- Read settings --------------------------------------------------

    readSettings();
    
    // -- camera controller -----------------------------------------------
    
    m_controller = new CameraController(this, model, port, path);

    connect(m_controller, SIGNAL(signalConnected(bool)),
            SLOT(slotConnected(bool)));
    connect(m_controller, SIGNAL(signalInfoMsg(const QString&)),
            m_status, SLOT(setText(const QString&)));
    connect(m_controller, SIGNAL(signalErrorMsg(const QString&)),
            SLOT(slotErrorMsg(const QString&)));
    connect(m_controller, SIGNAL(signalBusy(bool)),
            SLOT(slotBusy(bool)));
    connect(m_controller, SIGNAL(signalFolderList(const QStringList&)),
            SLOT(slotFolderList(const QStringList&)));
    connect(m_controller, SIGNAL(signalFileList(const GPItemInfoList&)),
            SLOT(slotFileList(const GPItemInfoList&)));
    connect(m_controller, SIGNAL(signalThumbnail(const QString&,
                                                 const QString&,
                                                 const QImage&)),
            SLOT(slotThumbnail(const QString&, const QString&,
                               const QImage&)));
    connect(m_controller, SIGNAL(signalDownloaded(const QString&, const QString&)),
            SLOT(slotDownloaded(const QString&, const QString&)));
    connect(m_controller, SIGNAL(signalSkipped(const QString&, const QString&)),
            SLOT(slotSkipped(const QString&, const QString&)));
    connect(m_controller, SIGNAL(signalDeleted(const QString&, const QString&)),
            SLOT(slotDeleted(const QString&, const QString&)));
    connect(m_cancelBtn, SIGNAL(clicked()),
            m_controller, SLOT(slotCancel()));

    m_busy = false;
    QTimer::singleShot(0, m_controller, SLOT(slotConnect()));
}

CameraUI::~CameraUI()
{
}

bool CameraUI::isBusy() const
{
    return m_busy;
}

void CameraUI::closeEvent(QCloseEvent* e)
{
    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run scanlib
    // over the folders we used. Bug: 119201
    ScanLib sLib;
    for (QStringList::iterator it = m_foldersToScan.begin(); 
         it != m_foldersToScan.end(); ++it)
    {
        kdDebug() << "Scanning " << (*it) << endl;
        sLib.findMissingItems( (*it) );
    }
    
    //---------------------------------------------------

    if(!m_lastDestURL.isEmpty())
        emit signalLastDestination(m_lastDestURL);

    delete m_controller;
    saveSettings();
    e->accept();
}


void CameraUI::slotHelp()
{
    KApplication::kApplication()->invokeHelp("camerainterface.anchor",
                                             "digikam");
}

void CameraUI::slotBusy(bool val)
{
    if (!val)
    {
        if (!m_busy)
            return;

        m_busy = false;
        m_cancelBtn->setEnabled(false);
        m_downloadBtn->setEnabled(true);
        m_deleteBtn->setEnabled(true);
        m_anim->stop();
        m_status->setText(i18n("Ready"));
        m_progress->hide();
    }
    else
    {
        if (m_busy)
            return;
        
        if (!m_anim->running())
            m_anim->start();
        m_busy = true;
        m_cancelBtn->setEnabled(true);
        m_downloadBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
    }
}

void CameraUI::slotConnected(bool val)
{
    if (!val)
    {
      if (KMessageBox::warningYesNo(this,
                                    i18n("Failed to connect to camera. "
                                         "Please make sure its connected "
                                         "properly and turned on. "
                                         "Would you like to try again?"), 
                                    i18n("Connection Failed"),
                                    i18n("Retry"),
                                    i18n("Abort"))
          == KMessageBox::Yes)
          QTimer::singleShot(0, m_controller, SLOT(slotConnect()));
      else
          close();
    }
    else
    {
        m_controller->listFolders();
    }
}

void CameraUI::slotFolderList(const QStringList& folderList)
{
    for (QStringList::const_iterator it = folderList.begin();
         it != folderList.end(); ++it)
    {
        m_controller->listFiles(*it);
    }
}

void CameraUI::slotFileList(const GPItemInfoList& fileList)
{
    for (GPItemInfoList::const_iterator it = fileList.begin();
         it != fileList.end(); ++it)
    {
        m_view->addItem(*it);
        m_controller->getThumbnail((*it).folder, (*it).name);
    }
}

void CameraUI::slotThumbnail(const QString& folder, const QString& file,
                             const QImage& thumbnail)
{
    m_view->setThumbnail(folder, file, thumbnail);    
}

void CameraUI::slotToggleAdvanced()
{
    m_showAdvanced = !m_showAdvanced;
    if (m_showAdvanced)
    {
        m_advBox->show();
        m_advBtn->setText(i18n("&Simple %1").arg("<<"));
    }
    else
    {
        m_advBox->hide();
        m_advBtn->setText(i18n("&Advanced %1").arg(">>"));
    }
}

void CameraUI::slotErrorMsg(const QString& msg)
{
    KMessageBox::error(this, msg);    
}

void CameraUI::slotDownloadSelected()
{
    slotDownload(true);
}

void CameraUI::slotDownloadAll()
{
    slotDownload(false);
}

void CameraUI::slotDownload(bool onlySelected)
{
    AlbumManager* man = AlbumManager::instance();

    Album* album = man->currentAlbum();
    if (album && album->type() != Album::PHYSICAL)
        album = 0;

    QString header(i18n("Select Destination Album for "
                        "Importing Camera Images"));

    QString newDirName;
    IconItem* firstItem = m_view->firstItem();
    if (firstItem)
    {
        CameraIconViewItem* iconItem =
            static_cast<CameraIconViewItem*>(firstItem);
        
        QDateTime date;
        date.setTime_t(iconItem->itemInfo()->mtime);
        newDirName = QString("%1, %2, %3")
                     .arg(KGlobal::locale()->calendar()->year(date.date()))
                     .arg(KGlobal::locale()->calendar()->monthName(date.date()))
                     .arg(KGlobal::locale()->calendar()->day(date.date()));
    }


    album = AlbumSelectDialog::selectAlbum(this,
                                           (PAlbum*)album,
                                           header,
                                           newDirName,
                                           m_autoAlbumCheck->isChecked());

    if (!album)
        return;

    KURL url;
    url.setPath(((PAlbum*)album)->folderPath());
    
    m_controller->downloadPrep();

    QString downloadName;
    QString name;
    QString folder;
    time_t  mtime;
    bool    autoRotate;
    bool    autoAlbum;

    autoRotate = m_autoRotateCheck->isChecked();
    autoAlbum  = m_autoAlbumCheck->isChecked();

    int total = 0;
    for (IconItem* item = m_view->firstItem(); item;
         item = item->nextItem())
    {
        if (onlySelected && !(item->isSelected()))
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        folder = iconItem->m_itemInfo->folder;
        name   = iconItem->m_itemInfo->name;
        downloadName = iconItem->getDownloadName();
        mtime        = iconItem->m_itemInfo->mtime;
        
        KURL u(url);
        if (autoAlbum)
        {
            QDateTime date;
            date.setTime_t(mtime);
            QString dirName(date.toString("yyyy-MM-dd"));
            QString errMsg;
            if (!createAutoAlbum(url, dirName, date.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(dirName);
            m_foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        else
        {
            m_foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        m_controller->download(folder, name, u.path(), autoRotate);
        addFileExtension(QFileInfo(u.path()).extension(false));
        total++;
    }

    if (total <= 0)
        return;
    
    m_lastDestURL = url;            
    m_progress->setProgress(0);
    m_progress->setTotalSteps(total);
    m_progress->show();
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = m_view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            QString folder = iconItem->m_itemInfo->folder;
            QString file   = iconItem->m_itemInfo->name;
            folders.append(folder);
            files.append(file);
            deleteList.append(folder + QString("/") + file);
        }
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18n("About to delete this image. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         "About to delete these %n images. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               i18n("Delete"))
        ==  KMessageBox::Continue) {

        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            m_controller->deleteFile(*itFolder, *itFile);
        }
    }
}

void CameraUI::slotDeleteAll()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = m_view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString folder = iconItem->m_itemInfo->folder;
        QString file   = iconItem->m_itemInfo->name;
        folders.append(folder);
        files.append(file);
        deleteList.append(folder + QString("/") + file);
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18n("About to delete this image. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         "About to delete these %n images. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               i18n("Delete"))
        ==  KMessageBox::Continue) {

        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            m_controller->deleteFile(*itFolder, *itFile);
        }
    }
}

void CameraUI::slotFileView(CameraIconViewItem* item)
{
    m_controller->openFile(item->itemInfo()->folder,
                           item->itemInfo()->name);
}

void CameraUI::slotFileProps(CameraIconViewItem* item)
{
    GPItemInfoDlg dlg(this, item->itemInfo());
    dlg.exec();
}

void CameraUI::slotFileExif(CameraIconViewItem* item)
{
    m_controller->getExif(item->itemInfo()->folder,
                          item->itemInfo()->name);
}

void CameraUI::slotItemsSelected(bool selected)
{
    m_downloadMenu->setItemEnabled(0, selected);
    m_deleteMenu->setItemEnabled(0, selected);
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file)
{
    CameraIconViewItem* iconItem = m_view->findItem(folder, file);
    if (iconItem)
    {
        iconItem->setDownloaded();
    }
    
    int curr = m_progress->progress();
    m_progress->setProgress(curr+1);
}

void CameraUI::slotSkipped(const QString&, const QString&)
{
    int curr = m_progress->progress();
    m_progress->setProgress(curr+1);
}

void CameraUI::slotDeleted(const QString& folder, const QString& file)
{
    m_view->removeItem(folder, file);
}

void CameraUI::readSettings()
{
    KConfig* config = kapp->config();

    int w, h;

    config->setGroup("Camera Settings");
    w = config->readNumEntry("Width", 500);
    h = config->readNumEntry("Height", 500);
    m_autoRotateCheck->setChecked(config->readBoolEntry("AutoRotate", true));
    m_autoAlbumCheck->setChecked(config->readBoolEntry("AutoAlbum", false));

    resize(w, h);
}

void CameraUI::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->writeEntry("AutoRotate", m_autoRotateCheck->isChecked());
    config->writeEntry("AutoAlbum", m_autoAlbumCheck->isChecked());
    config->sync();
}

bool CameraUI::createAutoAlbum(const KURL& parentURL,
                               const QString& name,
                               const QDate& date,
                               QString& errMsg)
{
    KURL u(parentURL);
    u.addPath(name);

    // first stat to see if the album exists
    struct stat buf;
    if (::stat(QFile::encodeName(u.path()), &buf) == 0)
    {
        // now check if its really a directory
        if (S_ISDIR(buf.st_mode))
            return true;
        else
        {
            errMsg = i18n("A file with same name (%1) exists in folder %2")
                     .arg(name)
                     .arg(parentURL.path());
            return false;
        }
    }

    // looks like the directory does not exist, try to create it

    AlbumManager* aman = AlbumManager::instance();
    PAlbum* parent =  aman->findPAlbum(parentURL);
    if (!parent)
    {
        errMsg = i18n("Failed to find Album for path '%1'")
                 .arg(parentURL.path());
        return false;
    }

    return aman->createPAlbum(parent, name, QString(""),
                              date, QString(""), errMsg);
}

void CameraUI::addFileExtension(const QString& ext)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;

    if (settings->getImageFileFilter().contains(ext) ||
        settings->getMovieFileFilter().contains(ext) ||
        settings->getAudioFileFilter().contains(ext) ||
        settings->getRawFileFilter().contains(ext))
        return;

    settings->setImageFileFilter(settings->getImageFileFilter() +
                                 QString(" *.") + ext);
    emit signalAlbumSettingsChanged();
}

}  // namespace Digikam

#include "cameraui.moc"

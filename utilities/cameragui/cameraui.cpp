/* ============================================================
 * File  : cameraui.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-16
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

#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <khelpmenu.h>

#include "albummanager.h"
#include "album.h"
#include "dirselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "gpiteminfodlg.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameraui.h"

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
    QPixmap icon = kapp->iconLoader()->loadIcon("stop",
                                                KIcon::NoGroup,
                                                22);
    m_cancelBtn->setText(i18n("Cancel"));
    m_cancelBtn->setIconSet(icon);
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
    
    m_advBtn      = new QPushButton(i18n("&Advanced %1").arg(">>"), this);
    m_helpBtn     = new QPushButton(i18n("&Help"), this);
    m_downloadBtn = new QPushButton(i18n("&Download"), this);
    m_deleteBtn   = new QPushButton(i18n("D&elete"), this);
    m_closeBtn    = new QPushButton(i18n("&Close"), this);
    
    btnLayout->addWidget(m_advBtn);
    btnLayout->addItem(new QSpacerItem(10,10,QSizePolicy::Expanding,
                                       QSizePolicy::Fixed));
    btnLayout->addWidget(m_helpBtn);
    btnLayout->addWidget(m_downloadBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_closeBtn);

    // About popupmenu button using a slot for calling the camera interface
    // anchor in Digikam handbook.
    
    KHelpMenu* helpMenu = new KHelpMenu(this, KApplication::kApplication()->aboutData(), false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Digikam Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpBtn->setPopup( helpMenu->menu() );

    mainLayout->addLayout(btnLayout);
    
    m_advBox = new QVBox(this);
    m_advBox->setSpacing(5);
    m_showAdvanced = false;
    m_advBox->hide();

    m_renameCustomizer = new RenameCustomizer(m_advBox);
    m_view->setRenameCustomizer(m_renameCustomizer);

    QVGroupBox* rotateBox = new QVGroupBox(i18n("Auto Orient"), m_advBox);
    m_autoRotateCheck = new QCheckBox(i18n("Automatically rotate/flip using "
                                           "camera provided information (EXIF)"),
                                      rotateBox);
    
    mainLayout->addWidget(m_advBox);
    
    m_downloadMenu = new QPopupMenu(this);
    m_downloadMenu->insertItem(i18n("Download Selected"),
                               this, SLOT(slotDownloadSelected()),
                               0, 0);
    m_downloadMenu->insertItem(i18n("Download All"),
                               this, SLOT(slotDownloadAll()),
                               0, 1);
    m_downloadMenu->setItemEnabled(0, false);
    m_downloadBtn->setPopup(m_downloadMenu);

    m_deleteMenu = new QPopupMenu(this);
    m_deleteMenu->insertItem(i18n("Delete Selected"),
                             this, SLOT(slotDeleteSelected()),
                             0, 0);
    m_deleteMenu->insertItem(i18n("Delete All"),
                             this, SLOT(slotDeleteAll()),
                             0, 1);
    m_deleteMenu->setItemEnabled(0, false);
    m_deleteBtn->setPopup(m_deleteMenu);

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
                                         "Would you like to try again?"))
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
    AlbumManager* man = AlbumManager::instance();
    QString libPath(man->getLibraryPath());
    QString currPath;

    Album* album = man->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        currPath = libPath;
    else
    {
        currPath = ((PAlbum*)album)->getKURL().path();
    }

    QString header(i18n("Select Destination Album for "
                        "Importing Camera Images"));
    KURL url = DirSelectDialog::selectDir(libPath, currPath,
                                          this, header);
    if (!url.isValid())
        return;
    
    m_controller->downloadPrep();

    QString downloadName;
    QString name;
    QString folder;
    bool    autoRotate;

    autoRotate = m_autoRotateCheck->isChecked();

    int total = 0;
    for (QIconViewItem* item = m_view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            folder = iconItem->m_itemInfo->folder;
            name   = iconItem->m_itemInfo->name;
            downloadName = iconItem->getDownloadName();

            KURL u(url);
            u.addPath(downloadName.isEmpty() ? name : downloadName);
            
            m_controller->download(folder, name, u.path(), autoRotate);
            total++;
        }
    }

    if (total <= 0)
        return;
    
    m_progress->setProgress(0);
    m_progress->setTotalSteps(total);
    m_progress->show();
}

void CameraUI::slotDownloadAll()
{
    AlbumManager* man = AlbumManager::instance();
    QString libPath(man->getLibraryPath());
    QString currPath;

    Album* album = man->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        currPath = libPath;
    else
    {
        currPath = ((PAlbum*)album)->getKURL().path();
    }

    QString header(i18n("Select Destination Album for "
                        "Importing Camera Images"));
    KURL url = DirSelectDialog::selectDir(libPath, currPath,
                                          this, header);
    if (!url.isValid())
        return;
    
    m_controller->downloadPrep();

    QString downloadName;
    QString name;
    QString folder;
    bool    autoRotate;

    autoRotate = m_autoRotateCheck->isChecked();

    int total = 0;
    for (QIconViewItem* item = m_view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        folder = iconItem->m_itemInfo->folder;
        name   = iconItem->m_itemInfo->name;
        downloadName = iconItem->getDownloadName();
        
        KURL u(url);
        u.addPath(downloadName.isEmpty() ? name : downloadName);
            
        m_controller->download(folder, name, u.path(), autoRotate);
        total++;
    }

    if (total <= 0)
        return;
    
    m_progress->setProgress(0);
    m_progress->setTotalSteps(total);
    m_progress->show();
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (QIconViewItem* item = m_view->firstItem(); item;
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
    
    for (QIconViewItem* item = m_view->firstItem(); item;
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

void CameraUI::slotDownloaded(const QString&, const QString&)
{
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

    resize(w, h);
}

void CameraUI::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->writeEntry("AutoRotate", m_autoRotateCheck->isChecked());
    config->sync();
}

#include "cameraui.moc"

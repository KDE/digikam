/* ============================================================
 * File  : cameraui.cpp
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

#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfileitem.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kpropertiesdialog.h>
#include <kiconloader.h>

#include <qmenubar.h>
#include <qlabel.h>
#include <qframe.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpopupmenu.h>

#include <guifactory.h>

#include "albummanager.h"
#include "album.h"
#include "cameraguiclient.h"
#include "dirselectdialog.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "thumbitem.h"
#include "cameracontroller.h"
#include "cameraui.h"

#include "camerapropsplugin.h"

class CameraUIPriv
{
public:

    CameraIconView*      view;
    CameraController*    controller;

    Digikam::GUIFactory* guiFactory;
    CameraGUIClient*     guiClient;

    QLabel*              statusLabel;
    QProgressBar*        progressBar;
    QTimer*              progressHideTimer;
};

CameraUI::CameraUI(QWidget* parent, const QString& model,
                   const QString& port, const QString& path)
    : QMainWindow(parent, model.latin1(), WType_TopLevel|WGroupLeader|WDestructiveClose)
{
    setCaption(model);
    
    d = new CameraUIPriv;  

    // -- build the gui -------------------------------------
    
    d->guiFactory = new Digikam::GUIFactory();
    d->guiClient  = new CameraGUIClient(this);
    d->guiFactory->insertClient(d->guiClient);
    d->guiFactory->buildGUI(this);

    // -- setup action connections ---------------------------

    connect(d->guiClient, SIGNAL(signalExit()),
            SLOT(close()));

    // -- setup the central widget ---------------------------

    QWidget* w       = new QWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(w, 0, 0);

    d->view = new CameraIconView(w);
    lay->addWidget(d->view);

    QFrame* s = new QFrame(w);
    s->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    QHBoxLayout* h = new QHBoxLayout(s, 5, 5);
    lay->addWidget(s);

    d->statusLabel = new QLabel(s);
    d->statusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                              QSizePolicy::Fixed, 2, 0));
    h->addWidget(d->statusLabel);

    d->progressBar = new QProgressBar(s);
    d->progressBar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                              QSizePolicy::Fixed, 1, 0));
    d->progressBar->setTotalSteps(100);
    h->addWidget(d->progressBar);
    d->progressBar->hide();

    d->progressHideTimer = new QTimer(this);
    
    setCentralWidget(w);

    // -- resize the window --------------------------

    loadInitialSize();
    
    // -- setup the camera ---------------------------

    d->controller = new CameraController(this, model,
                                         port, path);

    // -- setup connections --------------------------
    
    connect(d->view, SIGNAL(signalRightButtonClicked(ThumbItem *,
            const QPoint &)), this, 
            SLOT(slotRightButtonClicked(ThumbItem *,
            const QPoint &)));
    connect(d->view, SIGNAL(signalRightButtonClicked(const QPoint &)),
            this, SLOT(slotRightButtonClicked(const QPoint &)));

    connect(d->controller, SIGNAL(signalFatal(const QString&)),
            SLOT(slotFatal(const QString&)));
    
    connect(d->guiClient, SIGNAL(signalDownloadSelected()),
            SLOT(slotDownloadSelected()));
    connect(d->guiClient, SIGNAL(signalDownloadAll()),
            SLOT(slotDownloadAll()));
    
    connect(d->guiClient, SIGNAL(signalCancel()),
            d->controller, SLOT(slotCancel()));
    connect(d->controller, SIGNAL(signalBusy(bool)),
            SLOT(slotBusy(bool)));

    connect(d->guiClient, SIGNAL(signalCameraInfo()),
            d->controller, SLOT(slotCameraInfo()));
    
    connect(d->guiClient, SIGNAL(signalFileView()),
            this, SLOT(slotFileView()));
    connect(d->guiClient, SIGNAL(signalFileProps()),
            this, SLOT(slotFileProperties()));
    connect(d->guiClient, SIGNAL(signalFileExif()),
            this, SLOT(slotFileExif()));

    connect(d->guiClient, SIGNAL(signalSelectAll()),
            this, SLOT(slotSelectAll()));
    connect(d->guiClient, SIGNAL(signalSelectNone()),
            this, SLOT(slotSelectNone()));
    connect(d->guiClient, SIGNAL(signalSelectInvert()),
            this, SLOT(slotSelectInvert()));
    
    connect(d->view, SIGNAL(signalSelectionChanged()),
            SLOT(slotSelectionChanged()));
    connect(d->view, SIGNAL(signalFileView(CameraIconItem*)),
            SLOT(slotFileView(CameraIconItem*)));
    
    connect(d->controller, SIGNAL(signalNewItems(const KFileItemList&)),
            d->view, SLOT(slotNewItems(const KFileItemList&)));
    connect(d->controller, SIGNAL(signalThumbnail(const KFileItem*, const QPixmap&)),
            d->view, SLOT(slotGotThumbnail(const KFileItem*, const QPixmap&)));

    connect(d->controller, SIGNAL(signalInfoMessage(const QString&)),
            d->statusLabel, SLOT(setText(const QString&)));
    connect(d->controller, SIGNAL(signalInfoPercent(int)),
            SLOT(slotProgress(int)));

    connect(d->progressHideTimer, SIGNAL(timeout()),
            SLOT(slotProgressHide()));
}

CameraUI::~CameraUI()
{
    delete d->progressHideTimer;
    
    saveInitialSize();
    
    delete d->controller;
    delete d->guiClient;
    delete d->guiFactory;
    delete d;
}

void CameraUI::slotFatal(const QString& msg)
{
    KMessageBox::error( this, msg);
    close();
}

void CameraUI::slotBusy(bool val)
{
    d->guiClient->m_cancelAction->setEnabled(val);    
}

void CameraUI::slotSelectionChanged()
{
    bool selected = false;
    for (ThumbItem *item = d->view->firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
         {
             selected = true;
             break;
         }
    }

    d->guiClient->m_downloadSelAction->setEnabled(selected);
    d->guiClient->m_deleteSelAction->setEnabled(selected);
    d->guiClient->m_fileViewAction->setEnabled(selected);
    d->guiClient->m_filePropsAction->setEnabled(selected);
    d->guiClient->m_fileExifAction->setEnabled(selected);
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
    
    KURL url = DirSelectDialog::selectDir(libPath, currPath, this, header);
    if (!url.isValid())
        return;

    KFileItemList itemList;
    
    for (ThumbItem *item = d->view->firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
         {
             CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);
             itemList.append(iconItem->fileItem());
         }
    }

    if (itemList.isEmpty())
        return;
    
    d->controller->downloadSel(itemList, url.path());
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
    
    KURL url = DirSelectDialog::selectDir(libPath, currPath, this, header);
    if (!url.isValid())
        return;

    d->controller->downloadAll(url.path());
}

void CameraUI::slotProgress(int val)
{
    d->progressHideTimer->stop();
    if (d->progressBar->isHidden())
        d->progressBar->show();
    d->progressBar->setProgress(val);
    d->progressHideTimer->start(1000, false);
}

void CameraUI::slotProgressHide()
{
    d->progressBar->setProgress(0);
    d->progressBar->hide();
}

void CameraUI::slotFileView()
{
    slotFileView(d->view->firstSelectedItem());
}

void CameraUI::slotFileView(CameraIconItem* item)
{
    if (!item)
        return;

    d->controller->openFile(item->fileItem());
}

void CameraUI::slotFileProperties()
{
    slotFileProperties(d->view->firstSelectedItem());
}

void CameraUI::slotFileProperties(CameraIconItem* item)
{
    if (!item)
        return;

    KPropertiesDialog dlg(item->fileItem(), this, 0, true, false);
    CameraPropsPlugin *plugin = new CameraPropsPlugin(&dlg);
    dlg.insertPlugin(plugin);
    dlg.exec();
}

void CameraUI::slotFileExif()
{
    slotFileExif(d->view->firstSelectedItem());
}

void CameraUI::slotFileExif(CameraIconItem* item)
{
    if (!item)
        return;

    d->controller->getExif(item->fileItem());
}

void CameraUI::slotSelectAll()
{
    d->view->selectAll();    
}

void CameraUI::slotSelectNone()
{
    d->view->clearSelection();    
}

void CameraUI::slotSelectInvert()
{
    d->view->invertSelection();    
}

void CameraUI::loadInitialSize()
{
    KConfig *config = kapp->config();

    config->setGroup("Camera Settings");
    int w = config->readNumEntry("Width", 500);
    int h = config->readNumEntry("Height", 500);
    resize(w, h);
}

void CameraUI::saveInitialSize()
{
    KConfig *config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->sync();
}

void CameraUI::slotRightButtonClicked(ThumbItem *, const QPoint &pos)
{
    QPopupMenu popmenu(this);
    popmenu.insertItem(i18n("Download All"), 
                       this, SLOT(slotDownloadAll()));                           
    popmenu.insertItem(i18n("Download Selected"), 
                       this, SLOT(slotDownloadSelected()));
    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("editimage"),
                       i18n("View/Edit"),
                       this, SLOT(slotFileView()));
    popmenu.insertItem(SmallIcon("text_italic"), 
                       i18n("View Exif Information ..."),
                       this, SLOT(slotFileExif()));
    popmenu.insertItem(i18n("Properties ..."),
                       this, SLOT(slotFileProperties()));                       
    popmenu.exec(pos);
}

void CameraUI::slotRightButtonClicked(const QPoint &pos)
{
    QPopupMenu popmenu(this);
    popmenu.insertItem(i18n("Download All"), 
                       this, SLOT(slotDownloadAll()));                           
    popmenu.exec(pos);
}
#include "cameraui.moc"

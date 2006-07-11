/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-09-16
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
#include <qvbox.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kmessagebox.h>
#include <kprogress.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <khelpmenu.h>
#include <kdebug.h>
#include <kcalendarsystem.h>
#include <kurllabel.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "kdatetimeedit.h"
#include "sidebar.h"
#include "imagepropertiessidebarcamgui.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameraui.h"
#include "scanlib.h"

namespace Digikam
{

class CameraUIPriv
{
public:

    CameraUIPriv()
    {
        busy              = false;
        advBox            = 0;
        downloadMenu      = 0;
        deleteMenu        = 0;
        cancelBtn         = 0;
        splitter          = 0;
        rightSidebar      = 0;
        fixDateTimeCheck  = 0;
        autoRotateCheck   = 0;
        autoAlbumCheck    = 0;
        status            = 0;
        progress          = 0;
        controller        = 0;
        view              = 0;
        renameCustomizer  = 0;
        anim              = 0;
        dateTimeEdit      = 0;
        setPhotographerId = 0;
        setCredits        = 0;
        closed= false;
    }

    bool                          busy;
    bool                          closed;
    
    QStringList                   currentlyDeleting;
    QStringList                   foldersToScan;

    QPopupMenu                   *downloadMenu;
    QPopupMenu                   *deleteMenu;

    QToolButton                  *cancelBtn;

    QWidget                      *advBox;

    QCheckBox                    *autoRotateCheck;
    QCheckBox                    *autoAlbumCheck;
    QCheckBox                    *fixDateTimeCheck;
    QCheckBox                    *setPhotographerId;
    QCheckBox                    *setCredits;

    QSplitter                    *splitter;

    KProgress                    *progress;

    KSqueezedTextLabel           *status;

    KURL                          lastDestURL;

    KDateTimeEdit                *dateTimeEdit;
    
    CameraController             *controller;

    CameraIconView               *view;

    RenameCustomizer             *renameCustomizer;

    AnimWidget                   *anim;

    ImagePropertiesSideBarCamGui *rightSidebar;
};

CameraUI::CameraUI(QWidget* /*parent*/, const QString& cameraTitle,
                   const QString& model, const QString& port,
                   const QString& path)
        : KDialogBase(Plain, cameraTitle,
                      Help|User1|User2|User3|Close, Close,
                      0, // B.K.O # 116485: no parent for this modal dialog.
                      0, false, true,
                      i18n("D&elete"),
                      i18n("&Download"),
                      i18n("&Select"))
{
    d = new CameraUIPriv;
    setHelp("camerainterface.anchor", "digikam");

    // -------------------------------------------------------------------------
    
    QGridLayout* viewBoxLayout = new QGridLayout(plainPage(), 2, 5);
    viewBoxLayout->setColStretch( 0, 0 );
    viewBoxLayout->setColStretch( 1, 0 );
    viewBoxLayout->setColStretch( 2, 3 );
    viewBoxLayout->setColStretch( 3, 1 );
    viewBoxLayout->setColStretch( 4, 0 );
    viewBoxLayout->setColStretch( 5, 0 );

    QHBox* widget = new QHBox(plainPage());
    d->splitter   = new QSplitter(widget);
    d->view       = new CameraIconView(this, d->splitter);
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    d->view->setSizePolicy(rightSzPolicy);
        
    d->rightSidebar = new ImagePropertiesSideBarCamGui(widget, "CameraGui Sidebar Right", d->splitter,
                                                       Sidebar::Right, true);
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );       
    d->splitter->setOpaqueResize(false);
    
    // -------------------------------------------------------------------------

    d->advBox           = new QWidget(d->rightSidebar);
    QGridLayout* grid   = new QGridLayout( d->advBox, 3, 1, KDialog::marginHint());
    d->renameCustomizer = new RenameCustomizer(d->advBox);
    d->view->setRenameCustomizer(d->renameCustomizer);
        
    QVGroupBox* exifBox = new QVGroupBox(i18n("Use Camera Informations"), d->advBox);
    d->autoRotateCheck  = new QCheckBox(i18n("Rotate/flip image"), exifBox);
    d->autoAlbumCheck   = new QCheckBox(i18n("Date-based sub-albums"), exifBox);
    QWhatsThis::add( d->autoRotateCheck, i18n("<p>Toogle on this option if you want automatically "
                                              "rotated or flipped images using EXIF information "
                                              "provided by camera."));
    QWhatsThis::add( d->autoAlbumCheck, i18n("<p>Toogle on this option if you want downloaded photos "
                                             "into automatically created date-based sub-albums "
                                             "of destination album."));

    QVGroupBox* OnFlyBox = new QVGroupBox(i18n("On the Fly Operations"), d->advBox);
    d->setPhotographerId = new QCheckBox(i18n("Set default photographer identity"), OnFlyBox);
    d->setCredits        = new QCheckBox(i18n("Set default credit and copyright"), OnFlyBox);
    d->fixDateTimeCheck  = new QCheckBox(i18n("Fix internal date && time"), OnFlyBox);
    d->dateTimeEdit      = new KDateTimeEdit( OnFlyBox, "datepicker");
    
    QWhatsThis::add( d->setPhotographerId, i18n("<p>Toogle on this option to store default photographer identity "
                                                "into IPTC tags using main digiKam metadata settings."));
    QWhatsThis::add( d->setCredits, i18n("<p>Toogle on this option to store default credit and copyright information "
                                         "into IPTC tags using main digiKam metadata settings."));
    QWhatsThis::add( d->fixDateTimeCheck, i18n("<p>Toogle on this option to set date and time metadata "
                                               "tags to the right values if your camera don't set "
                                               "properly these tags when pictures are taken. The values will"
                                               "be saved in the DateTimeDigitized and DateTimeCreated EXIF/IPTC fields."));
                                               
    grid->addMultiCellWidget(d->renameCustomizer, 0, 0, 0, 1);
    grid->addMultiCellWidget(exifBox, 1, 1, 0, 1);
    grid->addMultiCellWidget(OnFlyBox, 2, 2, 0, 1);
    grid->setRowStretch(3, 10);

    d->rightSidebar->appendTab(d->advBox, SmallIcon("configure"), i18n("Settings"));
    
    // -------------------------------------------------------------------------
    
    viewBoxLayout->addMultiCellWidget(widget, 0, 0, 0, 5);
    viewBoxLayout->setRowSpacing(1, spacingHint());
    d->rightSidebar->loadViewState();
        
    // -------------------------------------------------------------------------

    d->cancelBtn = new QToolButton(plainPage());
    d->cancelBtn->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    d->cancelBtn->setPixmap( SmallIcon( "cancel" ) );
    d->cancelBtn->setEnabled(false);
    
    d->status   = new KSqueezedTextLabel(plainPage());
    d->progress = new KProgress(plainPage());
    d->progress->setMaximumHeight( fontMetrics().height() );
    d->progress->hide();

    QFrame *frame = new QFrame(plainPage());
    frame->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( frame );
    layout->setMargin( 2 );  // To make sure the frame gets displayed
    layout->setSpacing( 0 );

    KURLLabel *pixmapLogo = new KURLLabel( "http://www.digikam.org", QString::null, frame );
    pixmapLogo->setMargin(0);
    pixmapLogo->setScaledContents( false );
    pixmapLogo->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLogo->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    QToolTip::add(pixmapLogo, i18n("Visit digiKam project website"));
    KGlobal::dirs()->addResourceType("digikamlogo", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("digikamlogo", "digikamlogo.png");
    pixmapLogo->setPixmap( QPixmap( directory + "digikamlogo.png" ) );
    
    d->anim = new AnimWidget(frame, pixmapLogo->height());
    
    layout->addWidget( pixmapLogo );
    layout->addWidget( d->anim );

    viewBoxLayout->addMultiCellWidget(d->cancelBtn, 2, 2, 0, 0);
    viewBoxLayout->addMultiCellWidget(d->status, 2, 2, 2, 2);
    viewBoxLayout->addMultiCellWidget(d->progress, 2, 2, 3, 3);
    viewBoxLayout->addMultiCellWidget(frame, 2, 2, 5, 5);
    viewBoxLayout->setColSpacing(1, spacingHint());
    viewBoxLayout->setColSpacing(4, spacingHint());

    // -------------------------------------------------------------------------
    
    QPopupMenu* selectMenu = new QPopupMenu(this);
    selectMenu->insertItem(i18n("Select &All"),       d->view, SLOT(slotSelectAll()),    CTRL+Key_A);
    selectMenu->insertItem(i18n("Select N&one"),      d->view, SLOT(slotSelectNone()),   CTRL+Key_U);
    selectMenu->insertItem(i18n("&Invert Selection"), d->view, SLOT(slotSelectInvert()), CTRL+Key_Asterisk);
    selectMenu->insertSeparator();
    selectMenu->insertItem(i18n("Select &New Items"), d->view, SLOT(slotSelectNew()));
    actionButton(User3)->setPopup(selectMenu);    

    // -------------------------------------------------------------------------

    d->downloadMenu = new QPopupMenu(this);
    d->downloadMenu->insertItem(i18n("Download Selected"), this, SLOT(slotDownloadSelected()), 0, 0);
    d->downloadMenu->insertItem(i18n("Download All"), this, SLOT(slotDownloadAll()), 0, 1);
    d->downloadMenu->setItemEnabled(0, false);
    actionButton(User2)->setPopup(d->downloadMenu);

    // -------------------------------------------------------------------------
    
    d->deleteMenu = new QPopupMenu(this);
    d->deleteMenu->insertItem(i18n("Delete Selected"), this, SLOT(slotDeleteSelected()), 0, 0);
    d->deleteMenu->insertItem(i18n("Delete All"), this, SLOT(slotDeleteAll()), 0, 1);
    d->deleteMenu->setItemEnabled(0, false);
    actionButton(User1)->setPopup(d->deleteMenu);

    // -------------------------------------------------------------------------
    
    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));
            
    connect(pixmapLogo, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(slotProcessURL(const QString&)));     

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalSelected(CameraIconViewItem*, bool)),
            this, SLOT(slotItemsSelected(CameraIconViewItem*, bool)));

    connect(d->view, SIGNAL(signalFileView(CameraIconViewItem*)),
            this, SLOT(slotFileView(CameraIconViewItem*)));

    connect(d->view, SIGNAL(signalDownload()),
            this, SLOT(slotDownloadSelected()));

    connect(d->view, SIGNAL(signalDelete()),
            this, SLOT(slotDeleteSelected()));

    // -------------------------------------------------------------------------
    
    connect(d->rightSidebar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));
    
    connect(d->rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));
                
    connect(d->rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));                
    
    connect(d->rightSidebar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));                

    // -- Read settings --------------------------------------------------

    readSettings();
    
    // -- camera controller -----------------------------------------------
    
    d->controller = new CameraController(this, model, port, path);

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)));

    connect(d->controller, SIGNAL(signalInfoMsg(const QString&)),
            d->status, SLOT(setText(const QString&)));

    connect(d->controller, SIGNAL(signalErrorMsg(const QString&)),
            this, SLOT(slotErrorMsg(const QString&)));

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->controller, SIGNAL(signalFolderList(const QStringList&)),
            this, SLOT(slotFolderList(const QStringList&)));

    connect(d->controller, SIGNAL(signalFileList(const GPItemInfoList&)),
            this, SLOT(slotFileList(const GPItemInfoList&)));

    connect(d->controller, SIGNAL(signalThumbnail(const QString&, const QString&, const QImage&)),
            this, SLOT(slotThumbnail(const QString&, const QString&, const QImage&)));

    connect(d->controller, SIGNAL(signalDownloaded(const QString&, const QString&)),
            this, SLOT(slotDownloaded(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalSkipped(const QString&, const QString&)),
            this, SLOT(slotSkipped(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalDeleted(const QString&, const QString&)),
            this, SLOT(slotDeleted(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalExifFromFile(const QString&, const QString&)),
            this, SLOT(slotExifFromFile(const QString&, const QString&)));
    
    connect(d->controller, SIGNAL(signalExifData(const QByteArray&)),
            this, SLOT(slotExifFromData(const QByteArray&)));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SLOT(slotCancelButton()));

    d->view->setFocus();
    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

CameraUI::~CameraUI()
{
    delete d->rightSidebar;
    delete d->controller;
    delete d;
}

void CameraUI::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Camera Settings");
    d->autoRotateCheck->setChecked(config->readBoolEntry("AutoRotate", true));
    d->autoAlbumCheck->setChecked(config->readBoolEntry("AutoAlbum", false));
    d->fixDateTimeCheck->setChecked(config->readBoolEntry("FixDateTime", false));
    d->setPhotographerId->setChecked(config->readBoolEntry("SetPhotographerId", false));
    d->setCredits->setChecked(config->readBoolEntry("SetCredits", false));
    
    if(config->hasKey("Splitter Sizes"))
        d->splitter->setSizes(config->readIntListEntry("Splitter Sizes"));

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    
    resize(configDialogSize("Camera Settings"));
}

void CameraUI::saveSettings()
{
    saveDialogSize("Camera Settings");

    KConfig* config = kapp->config();
    config->setGroup("Camera Settings");
    config->writeEntry("AutoRotate", d->autoRotateCheck->isChecked());
    config->writeEntry("AutoAlbum", d->autoAlbumCheck->isChecked());
    config->writeEntry("FixDateTime", d->fixDateTimeCheck->isChecked());
    config->writeEntry("SetPhotographerId", d->setPhotographerId->isChecked());
    config->writeEntry("SetCredits", d->setCredits->isChecked());
    config->writeEntry("Splitter Sizes", d->splitter->sizes());
    config->sync();
}

void CameraUI::slotProcessURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

bool CameraUI::isBusy() const
{
    return d->busy;
}

bool CameraUI::isClosed() const
{
    return d->closed;
}

void CameraUI::slotCancelButton()
{
    d->status->setText(i18n("Cancelling current operation, please wait..."));
    d->progress->hide();
    QTimer::singleShot(0, d->controller, SLOT(slotCancel()));
    d->currentlyDeleting.clear();
}

void CameraUI::closeEvent(QCloseEvent* e)
{
    if (dialogClosed())
        e->accept();
    else
        e->ignore();
}

void CameraUI::slotClose()
{
    if (dialogClosed())
        reject();
}

bool CameraUI::dialogClosed()
{
    if (d->closed)
        return true;

    if (isBusy())
    {
        if (KMessageBox::questionYesNo(this,
                                       i18n("Do you want to close the dialog "
                                            "and cancel the current operation?"))
            == KMessageBox::No)
            return false;
    }

    d->status->setText(i18n("Disconnecting from camera, please Wait..."));
    d->progress->hide();

    if (isBusy())
    {
        d->controller->slotCancel();
        // will be read in slotBusy later and finishDialog
        // will be called only when everything is finished
        d->closed = true;
    }
    else
    {
        finishDialog();
        d->closed = true;
    }

    return true;
}

void CameraUI::finishDialog()
{
    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run scanlib
    // over the folders we used. Bug: 119201

    ScanLib sLib;
    for (QStringList::iterator it = d->foldersToScan.begin();
         it != d->foldersToScan.end(); ++it)
    {
        //kdDebug() << "Scanning " << (*it) << endl;
        sLib.findMissingItems( (*it) );
    }

    // Never call finalScan after deleteLater() - ScanLib will call processEvent(),
    // and the delete event may be executed!
    deleteLater();

    if(!d->lastDestURL.isEmpty())
        emit signalLastDestination(d->lastDestURL);

    saveSettings();
}

void CameraUI::slotBusy(bool val)
{
    if (!val)
    {
        if (!d->busy)
            return;

        d->busy = false;
        d->cancelBtn->setEnabled(false);
        d->advBox->setEnabled(true);
        enableButton(User2, true);
        enableButton(User1, true);
        d->anim->stop();
        d->status->setText(i18n("Ready"));
        d->progress->hide();

        // like WDestructiveClose, but after camera controller operation has safely finished
        if (d->closed)
        {
            finishDialog();
        }
    }
    else
    {
        if (d->busy)
            return;

        if (!d->anim->running())
            d->anim->start();

        d->busy = true;
        d->cancelBtn->setEnabled(true);
        d->advBox->setEnabled(false);
        enableButton(User2, false);
        enableButton(User1, false);
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
          QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
      else
          close();
    }
    else
    {
        d->controller->listFolders();
    }
}

void CameraUI::slotFolderList(const QStringList& folderList)
{
    if (d->closed)
        return;

    for (QStringList::const_iterator it = folderList.begin();
         it != folderList.end(); ++it)
    {
        d->controller->listFiles(*it);
    }
}

void CameraUI::slotFileList(const GPItemInfoList& fileList)
{
    if (d->closed)
        return;

    for (GPItemInfoList::const_iterator it = fileList.begin();
         it != fileList.end(); ++it)
    {
        d->view->addItem(*it);
        d->controller->getThumbnail((*it).folder, (*it).name);
    }
}

void CameraUI::slotThumbnail(const QString& folder, const QString& file,
                             const QImage& thumbnail)
{
    d->view->setThumbnail(folder, file, thumbnail);
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
    IconItem* firstItem = d->view->firstItem();
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

    album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header, newDirName,
                                           d->autoAlbumCheck->isChecked());

    if (!album)
        return;

    KURL url;
    url.setPath(((PAlbum*)album)->folderPath());
    
    d->controller->downloadPrep();

    QString author, authorTitle, credit, source, copyright;
    QString downloadName;
    QString name;
    QString folder;
    time_t  mtime;
    int     total = 0;
    
    bool autoRotate        = d->autoRotateCheck->isChecked();
    bool autoAlbum         = d->autoAlbumCheck->isChecked();
    bool fixDateTime       = d->fixDateTimeCheck->isChecked();
    QDateTime newDateTime  = d->dateTimeEdit->dateTime();
    bool setPhotographerId = d->setPhotographerId->isChecked();
    bool setCredits        = d->setCredits->isChecked();
    
    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        author      = settings->getIptcAuthor();
        authorTitle = settings->getIptcAuthorTitle();
        credit      = settings->getIptcCredit();
        source      = settings->getIptcSource();
        copyright   = settings->getIptcCopyright();        
    }
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        if (onlySelected && !(item->isSelected()))
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        folder       = iconItem->m_itemInfo->folder;
        name         = iconItem->m_itemInfo->name;
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
            d->foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        else
        {
            d->foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        
        d->controller->download(folder, name, u.path(), autoRotate, fixDateTime, newDateTime,
                                setPhotographerId, author, authorTitle, 
                                setCredits, credit, source, copyright);
        addFileExtension(QFileInfo(u.path()).extension(false));
        total++;
    }

    if (total <= 0)
        return;
    
    d->lastDestURL = url;
    d->progress->setProgress(0);
    d->progress->setTotalSteps(total);
    d->progress->show();
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = d->view->firstItem(); item;
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
        ==  KMessageBox::Continue) 
    {

        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            // the currentlyDeleting list is used to prevent loading items which
            // will immenently be deleted into the sidebar and wasting time
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotDeleteAll()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = d->view->firstItem(); item;
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
            d->controller->deleteFile(*itFolder, *itFile);
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotFileView(CameraIconViewItem* item)
{
    d->controller->openFile(item->itemInfo()->folder,
                           item->itemInfo()->name);
}

void CameraUI::slotExifFromFile(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->view->findItem(folder, file);

    if (!item)
        return;

    // We will trying to load exif data from THM file (thumbnail) if exist,
    // especially provided by recent USM camera.
    // If no THM file is availalble, we will trying to get Exif data from real image file.

    QFileInfo fi(folder + QString("/") + file);
    QFileInfo thmLo(fi.dirPath() + "/" + fi.baseName() + ".thm");          // Lowercase
    QFileInfo thmUp(fi.dirPath() + "/" + fi.baseName() + ".THM");          // Uppercase

    if (thmLo.exists())
    {
        d->rightSidebar->itemChanged(item->itemInfo(), KURL(thmLo.filePath()), 
                                     QByteArray(), d->view, item);
    }
    else if (thmUp.exists())
    {
        d->rightSidebar->itemChanged(item->itemInfo(), KURL(thmUp.filePath()), 
                                     QByteArray(), d->view, item);
    }
    else
    {
        d->rightSidebar->itemChanged(item->itemInfo(), KURL(fi.filePath()), 
                                     QByteArray(), d->view, item);
    }
}

void CameraUI::slotExifFromData(const QByteArray& exifData)
{
    CameraIconViewItem* item = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());

    if (!item)
        return;

    KURL url(item->itemInfo()->folder + "/" + item->itemInfo()->name);

    // Sometimes, GPhoto2 drivers return complete APP1 JFIF section. Exiv2 cannot 
    // decode (yet) exif metadata from APP1. We will find Exif header to get data at this place 
    // to please with Exiv2...

    kdDebug() << "Size of Exif metadata from camera = " << exifData.size() << endl;
    char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
    
    if (!exifData.isEmpty())
    {
        int i = exifData.find(*exifHeader);
        if (i != -1)
        {
            kdDebug() << "Exif header found at position " << i << endl;
            i = i + sizeof(exifHeader);
            QByteArray data(exifData.size()-i);
            memcpy(data.data(), exifData.data()+i, data.size());
            d->rightSidebar->itemChanged(item->itemInfo(), url, data, d->view, item);
            return;
        }
    }

    d->rightSidebar->itemChanged(item->itemInfo(), url, exifData, d->view, item);
}

void CameraUI::slotItemsSelected(CameraIconViewItem* item, bool selected)
{
    d->downloadMenu->setItemEnabled(0, selected);
    d->deleteMenu->setItemEnabled(0, selected);

    if (selected)
    {
        // if selected item is in the list of item which will be deleted, set no current item
        if (d->currentlyDeleting.find(item->itemInfo()->folder + item->itemInfo()->name)
             == d->currentlyDeleting.end())
        {
            KURL url(item->itemInfo()->folder + "/" + item->itemInfo()->name);
            d->rightSidebar->itemChanged(item->itemInfo(), url, QByteArray(), d->view, item);
            d->controller->getExif(item->itemInfo()->folder, item->itemInfo()->name);
        }
        else
        {
            d->rightSidebar->slotNoCurrentItem();
        }
    }
    else
        d->rightSidebar->slotNoCurrentItem();
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
    {
        iconItem->setDownloaded();

        if (iconItem->isSelected())
            slotItemsSelected(iconItem, true);
    }
    
    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotSkipped(const QString&, const QString&)
{
    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotDeleted(const QString& folder, const QString& file)
{
    d->view->removeItem(folder, file);
    // do this after removeItem, which will signal to slotItemsSelected, which checks for the list
    d->currentlyDeleting.remove(folder + file);
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

    return aman->createPAlbum(parent, name, QString(""), date, QString(""), errMsg);
}

void CameraUI::addFileExtension(const QString& ext)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;

    if (settings->getImageFileFilter().upper().contains(ext.upper()) ||
        settings->getMovieFileFilter().upper().contains(ext.upper()) ||
        settings->getAudioFileFilter().upper().contains(ext.upper()) ||
        settings->getRawFileFilter().upper().contains(ext.upper()))
        return;

    settings->setImageFileFilter(settings->getImageFileFilter() + QString(" *.") + ext);
    emit signalAlbumSettingsChanged();
}

void CameraUI::slotFirstItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->firstItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

void CameraUI::slotPrevItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem)
       d->view->setCurrentItem(currItem->prevItem());
}

void CameraUI::slotNextItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem->nextItem());
}

void CameraUI::slotLastItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->lastItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

}  // namespace Digikam

#include "cameraui.moc"

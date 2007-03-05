/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
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

#define CAMERA_INFO_MENU_ID 255

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
#include <qhbox.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qtoolbox.h>
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

#include <kfiledialog.h>
#include <kimageio.h>
#include <kaboutdata.h>
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
#include <kcalendarsystem.h>
#include <kurllabel.h>
#include <ksqueezedtextlabel.h>

#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes.

#include "ddebug.h"
#include "thumbnailsize.h"
#include "kdatetimeedit.h"
#include "sidebar.h"
#include "scanlib.h"
#include "downloadsettingscontainer.h"
#include "imagepropertiessidebarcamgui.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "camerafolderdialog.h"
#include "camerainfodialog.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameraui.h"
#include "cameraui.moc"

namespace Digikam
{

class CameraUIPriv
{
public:

    enum SettingsTab
    {
        RENAMEFILEPAGE=0,
        AUTOALBUMPAGE,
        ONFLYPAGE
    };

    enum DateFormatOptions
    {
        IsoDateFormat=0,
        TextDateFormat,
        LocalDateFormat
    };

    CameraUIPriv()
    {
        busy               = false;
        closed             = false;
        helpMenu           = 0;
        advBox             = 0;
        downloadMenu       = 0;
        deleteMenu         = 0;
        imageMenu          = 0;
        cancelBtn          = 0;
        splitter           = 0;
        rightSidebar       = 0;
        fixDateTimeCheck   = 0;
        autoRotateCheck    = 0;
        autoAlbumDateCheck = 0;
        autoAlbumExtCheck  = 0;
        status             = 0;
        progress           = 0;
        controller         = 0;
        view               = 0;
        renameCustomizer   = 0;
        anim               = 0;
        dateTimeEdit       = 0;
        setPhotographerId  = 0;
        setCredits         = 0;
        losslessFormat     = 0;
        convertJpegCheck   = 0;
        formatLabel        = 0;
        folderDateLabel    = 0;
        folderDateFormat   = 0;
    }

    bool                          busy;
    bool                          closed;
    
    QString                       cameraTitle;

    QStringList                   currentlyDeleting;
    QStringList                   foldersToScan;
    QStringList                   cameraFolderList;

    QPopupMenu                   *downloadMenu;
    QPopupMenu                   *deleteMenu;
    QPopupMenu                   *imageMenu;
    
    QToolButton                  *cancelBtn;

    QToolBox                     *advBox;

    QCheckBox                    *autoRotateCheck;
    QCheckBox                    *autoAlbumDateCheck;
    QCheckBox                    *autoAlbumExtCheck;
    QCheckBox                    *fixDateTimeCheck;
    QCheckBox                    *setPhotographerId;
    QCheckBox                    *setCredits;
    QCheckBox                    *convertJpegCheck;

    QLabel                       *formatLabel;
    QLabel                       *folderDateLabel;

    QComboBox                    *losslessFormat;
    QComboBox                    *folderDateFormat;

    QSplitter                    *splitter;

    QDateTime                     lastAccess;

    KProgress                    *progress;

    KSqueezedTextLabel           *status;

    KURL                          lastDestURL;

    KHelpMenu                    *helpMenu;

    KDateTimeEdit                *dateTimeEdit;
    
    CameraController             *controller;

    CameraIconView               *view;

    RenameCustomizer             *renameCustomizer;

    AnimWidget                   *anim;

    ImagePropertiesSideBarCamGui *rightSidebar;
};

CameraUI::CameraUI(QWidget* /*parent*/, const QString& cameraTitle, 
                   const QString& model, const QString& port,
                   const QString& path, const QDateTime lastAccess)
        : KDialogBase(Plain, cameraTitle,
                      Help|User1|User2|User3|Close, Close,
                      0, // B.K.O # 116485: no parent for this modal dialog.
                      0, false, true,
                      i18n("D&elete"),
                      i18n("&Download"),
                      i18n("&Images"))
{
    d = new CameraUIPriv;
    d->lastAccess  = lastAccess;
    d->cameraTitle = cameraTitle;
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

    d->advBox            = new QToolBox(d->rightSidebar);
    d->renameCustomizer  = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->view->setRenameCustomizer(d->renameCustomizer);

    QWhatsThis::add( d->advBox, i18n("<p>Set how digiKam will rename picture files as they are downloaded."));

    d->advBox->insertItem(CameraUIPriv::RENAMEFILEPAGE, d->renameCustomizer, 
                          SmallIconSet("fileimport"), i18n("File Renaming Options"));
        
    // -- Albums Auto-creation options -----------------------------------------

    QWidget* albumBox      = new QWidget(d->advBox);
    QVBoxLayout* albumVlay = new QVBoxLayout(albumBox, marginHint(), spacingHint());
    d->autoAlbumExtCheck   = new QCheckBox(i18n("Extension-based sub-albums"), albumBox);
    d->autoAlbumDateCheck  = new QCheckBox(i18n("Date-based sub-albums"), albumBox);
    QHBox *hbox1           = new QHBox(albumBox);
    d->folderDateLabel     = new QLabel(i18n("Date format:"), hbox1);
    d->folderDateFormat    = new QComboBox(hbox1);
    d->folderDateFormat->insertItem(i18n("ISO"),            CameraUIPriv::IsoDateFormat);
    d->folderDateFormat->insertItem(i18n("Full Text"),      CameraUIPriv::TextDateFormat);
    d->folderDateFormat->insertItem(i18n("Local Settings"), CameraUIPriv::LocalDateFormat);
    albumVlay->addWidget(d->autoAlbumExtCheck);
    albumVlay->addWidget(d->autoAlbumDateCheck);
    albumVlay->addWidget(hbox1);
    albumVlay->addStretch();

    QWhatsThis::add( albumBox, i18n("<p>Set how digiKam creates albums automatically when downloading."));
    QWhatsThis::add( d->autoAlbumExtCheck, i18n("<p>Toggle on this option if you want to download your "
                     "pictures into automatically created file extension-based sub-albums of the destination "
                     "album. This way, you can separate JPEG and RAW files as they are downloaded from your camera."));
    QWhatsThis::add( d->autoAlbumDateCheck, i18n("<p>Toggle on this option if you want to "
                     "download your pictures into automatically created file date-based sub-albums "
                     "of the destination album."));
    QWhatsThis::add( d->folderDateFormat, i18n("<p>Select here your preferred date format used to "
                     "create new albums. The options available are:<p>"
                     "<b>ISO</b>: the date format is in accordance with ISO 8601 "
                     "(YYYY-MM-DD). E.g.: <i>2006-08-24</i><p>"
                     "<b>Full Text</b>: the date format is in a user-readable string. "
                     "E.g.: <i>Thu Aug 24 2006</i><p>"
                     "<b>Local Settings</b>: the date format depending on KDE control panel settings.<p>"));

    d->advBox->insertItem(CameraUIPriv::AUTOALBUMPAGE, albumBox, SmallIconSet("folder_new"), 
                          i18n("Auto-creation of Albums"));

    // -- On the Fly options ---------------------------------------------------

    QWidget* onFlyBox      = new QWidget(d->advBox);
    QVBoxLayout* onFlyVlay = new QVBoxLayout(onFlyBox, marginHint(), spacingHint());
    d->setPhotographerId   = new QCheckBox(i18n("Set default photographer identity"), onFlyBox);
    d->setCredits          = new QCheckBox(i18n("Set default credit and copyright"), onFlyBox);
    d->fixDateTimeCheck    = new QCheckBox(i18n("Fix internal date && time"), onFlyBox);
    d->dateTimeEdit        = new KDateTimeEdit(onFlyBox, "datepicker");
    d->autoRotateCheck     = new QCheckBox(i18n("Auto-rotate/flip image"), onFlyBox);
    d->convertJpegCheck    = new QCheckBox(i18n("Convert to lossless file format"), onFlyBox);
    QHBox *hbox2           = new QHBox(onFlyBox);
    d->formatLabel         = new QLabel(i18n("New image format:"), hbox2);
    d->losslessFormat      = new QComboBox(hbox2);
    d->losslessFormat->insertItem("PNG", 0);
    onFlyVlay->addWidget(d->setPhotographerId);
    onFlyVlay->addWidget(d->setCredits);
    onFlyVlay->addWidget(d->fixDateTimeCheck);
    onFlyVlay->addWidget(d->dateTimeEdit);
    onFlyVlay->addWidget(d->autoRotateCheck);
    onFlyVlay->addWidget(d->convertJpegCheck);
    onFlyVlay->addWidget(hbox2);
    onFlyVlay->addStretch();

    QWhatsThis::add( onFlyBox, i18n("<p>Set here all options to fix/transform JPEG files automatically "
                     "as they are downloaded."));
    QWhatsThis::add( d->autoRotateCheck, i18n("<p>Toggle on this option if you want images automatically "
                     "rotated or flipped using EXIF information provided by the camera."));
    QWhatsThis::add( d->setPhotographerId, i18n("<p>Toggle on this option to store the default "
                     "photographer identity into IPTC tags using digiKam's metadata settings."));
    QWhatsThis::add( d->setCredits, i18n("<p>Toggle on this option to store the default credit "
                     "and copyright information into IPTC tags using digiKam's metadata settings."));
    QWhatsThis::add( d->fixDateTimeCheck, i18n("<p>Toggle on this option to set date and time metadata "
                     "tags to the right values if your camera does not set "
                     "these tags correctly when pictures are taken. The values will "
                     "be saved in the DateTimeDigitized and DateTimeCreated EXIF/IPTC fields."));
    QWhatsThis::add( d->convertJpegCheck, i18n("<p>Toggle on this option to automatically convert "
                     "all JPEG files to a lossless image format. <b>Note:</b> Image conversion can take a "
                     "while on a slow computer."));
    QWhatsThis::add( d->losslessFormat, i18n("<p>Select your preferred lossless image file format to "
                     "convert to.  <b>Note:</b> All metadata will be preserved during the conversion."));

    d->advBox->insertItem(CameraUIPriv::ONFLYPAGE, onFlyBox, SmallIconSet("run"), 
                          i18n("On the Fly Operations (JPEG only)"));
                                               
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

    KURLLabel *pixmapLogo = new KURLLabel( "http://www.digikam.org", QString(), frame );
    pixmapLogo->setMargin(0);
    pixmapLogo->setScaledContents( false );
    pixmapLogo->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLogo->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    QToolTip::add(pixmapLogo, i18n("Visit digiKam project website"));
    KGlobal::dirs()->addResourceType("logo-digikam", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-digikam", "logo-digikam.png");
    pixmapLogo->setPixmap( QPixmap( directory + "logo-digikam.png" ) );
    
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
    
    d->imageMenu = new QPopupMenu(this);
    d->imageMenu->insertItem(i18n("Select &All"),       d->view, SLOT(slotSelectAll()),    CTRL+Key_A, 0);
    d->imageMenu->insertItem(i18n("Select N&one"),      d->view, SLOT(slotSelectNone()),   CTRL+Key_U, 1);
    d->imageMenu->insertItem(i18n("&Invert Selection"), d->view, SLOT(slotSelectInvert()), CTRL+Key_Asterisk, 2);
    d->imageMenu->insertSeparator();
    d->imageMenu->insertItem(i18n("Select &New Items"), d->view, SLOT(slotSelectNew()), 0, 3);
    d->imageMenu->insertSeparator();
    d->imageMenu->insertItem(i18n("Increase Thumbs"),   this,    SLOT(slotIncreaseThumbSize()), CTRL+Key_Plus, 4);
    d->imageMenu->insertItem(i18n("Decrease Thumbs"),   this,    SLOT(slotDecreaseThumbSize()), CTRL+Key_Minus, 5);
    d->imageMenu->insertSeparator();
    d->imageMenu->insertItem(i18n("Toggle Lock"),       this,    SLOT(slotToggleLock()), 0, 6);
    actionButton(User3)->setPopup(d->imageMenu);    

    // -------------------------------------------------------------------------

    d->downloadMenu = new QPopupMenu(this);
    d->downloadMenu->insertItem(i18n("Download Selected"), this, SLOT(slotDownloadSelected()), 0, 0);
    d->downloadMenu->insertItem(i18n("Download All"),      this, SLOT(slotDownloadAll()), 0, 1);
    d->downloadMenu->insertSeparator();
    d->downloadMenu->insertItem(i18n("Upload..."),         this, SLOT(slotUpload()), 0, 2);
    d->downloadMenu->setItemEnabled(0, false);
    actionButton(User2)->setPopup(d->downloadMenu);

    // -------------------------------------------------------------------------
    
    d->deleteMenu = new QPopupMenu(this);
    d->deleteMenu->insertItem(i18n("Delete Selected"), this, SLOT(slotDeleteSelected()), 0, 0);
    d->deleteMenu->insertItem(i18n("Delete All"),      this, SLOT(slotDeleteAll()), 0, 1);
    d->deleteMenu->setItemEnabled(0, false);
    actionButton(User1)->setPopup(d->deleteMenu);

    // -------------------------------------------------------------------------

    QPushButton *helpButton = actionButton( Help );
    d->helpMenu = new KHelpMenu(this, kapp->aboutData(), false);
    d->helpMenu->menu()->insertItem(SmallIcon("camera"), i18n("Camera Information"), 
                                    this, SLOT(slotInformations()), 0, CAMERA_INFO_MENU_ID, 0);
    helpButton->setPopup( d->helpMenu->menu() );

    // -------------------------------------------------------------------------
    
    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateFormat, SLOT(setEnabled(bool)));

    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->losslessFormat, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->formatLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->view, SLOT(slotDownloadNameChanged()));

    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));
            
    connect(pixmapLogo, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(slotProcessURL(const QString&)));     

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalSelected(CameraIconViewItem*, bool)),
            this, SLOT(slotItemsSelected(CameraIconViewItem*, bool)));

    connect(d->view, SIGNAL(signalFileView(CameraIconViewItem*)),
            this, SLOT(slotFileView(CameraIconViewItem*)));

    connect(d->view, SIGNAL(signalUpload(const KURL::List&)),
            this, SLOT(slotUploadItems(const KURL::List&)));

    connect(d->view, SIGNAL(signalDownload()),
            this, SLOT(slotDownloadSelected()));

    connect(d->view, SIGNAL(signalDelete()),
            this, SLOT(slotDeleteSelected()));

    connect(d->view, SIGNAL(signalToggleLock()),
            this, SLOT(slotToggleLock()));

    connect(d->view, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotNewSelection(bool)));

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
    
    d->controller = new CameraController(this, d->cameraTitle, model, port, path);

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)));

    connect(d->controller, SIGNAL(signalInfoMsg(const QString&)),
            d->status, SLOT(setText(const QString&)));

    connect(d->controller, SIGNAL(signalErrorMsg(const QString&)),
            this, SLOT(slotErrorMsg(const QString&)));

    connect(d->controller, SIGNAL(signalCameraInformations(const QString&, const QString&, const QString&)),
            this, SLOT(slotCameraInformations(const QString&, const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->controller, SIGNAL(signalFolderList(const QStringList&)),
            this, SLOT(slotFolderList(const QStringList&)));

    connect(d->controller, SIGNAL(signalFileList(const GPItemInfoList&)),
            this, SLOT(slotFileList(const GPItemInfoList&)));

    connect(d->controller, SIGNAL(signalThumbnail(const QString&, const QString&, const QImage&)),
            this, SLOT(slotThumbnail(const QString&, const QString&, const QImage&)));

    connect(d->controller, SIGNAL(signalDownloaded(const QString&, const QString&, int)),
            this, SLOT(slotDownloaded(const QString&, const QString&, int)));

    connect(d->controller, SIGNAL(signalSkipped(const QString&, const QString&)),
            this, SLOT(slotSkipped(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalDeleted(const QString&, const QString&, bool)),
            this, SLOT(slotDeleted(const QString&, const QString&, bool)));

    connect(d->controller, SIGNAL(signalLocked(const QString&, const QString&, bool)),
            this, SLOT(slotLocked(const QString&, const QString&, bool)));

    connect(d->controller, SIGNAL(signalExifFromFile(const QString&, const QString&)),
            this, SLOT(slotExifFromFile(const QString&, const QString&)));
    
    connect(d->controller, SIGNAL(signalExifData(const QByteArray&)),
            this, SLOT(slotExifFromData(const QByteArray&)));

    connect(d->controller, SIGNAL(signalUploaded(const GPItemInfo&)),
            this, SLOT(slotUploaded(const GPItemInfo&)));

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
    d->advBox->setCurrentIndex(config->readNumEntry("Settings Tab", CameraUIPriv::RENAMEFILEPAGE));
    d->autoRotateCheck->setChecked(config->readBoolEntry("AutoRotate", true));
    d->autoAlbumDateCheck->setChecked(config->readBoolEntry("AutoAlbumDate", false));
    d->autoAlbumExtCheck->setChecked(config->readBoolEntry("AutoAlbumExt", false));
    d->fixDateTimeCheck->setChecked(config->readBoolEntry("FixDateTime", false));
    d->setPhotographerId->setChecked(config->readBoolEntry("SetPhotographerId", false));
    d->setCredits->setChecked(config->readBoolEntry("SetCredits", false));
    d->convertJpegCheck->setChecked(config->readBoolEntry("ConvertJpeg", false));
    d->losslessFormat->setCurrentItem(config->readNumEntry("LossLessFormat", 0));   // PNG by default
    d->folderDateFormat->setCurrentItem(config->readNumEntry("FolderDateFormat", CameraUIPriv::IsoDateFormat));

    d->view->setThumbnailSize(ThumbnailSize((ThumbnailSize::Size)config->readNumEntry("ThumbnailSize", 
                              ThumbnailSize::Large)));
    
    if(config->hasKey("Splitter Sizes"))
        d->splitter->setSizes(config->readIntListEntry("Splitter Sizes"));

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    d->losslessFormat->setEnabled(convertLosslessJpegFiles());
    d->formatLabel->setEnabled(convertLosslessJpegFiles());
    d->folderDateFormat->setEnabled(d->autoAlbumDateCheck->isChecked());
    d->folderDateLabel->setEnabled(d->autoAlbumDateCheck->isChecked());

    resize(configDialogSize("Camera Settings"));
}

void CameraUI::saveSettings()
{
    saveDialogSize("Camera Settings");

    KConfig* config = kapp->config();
    config->setGroup("Camera Settings");
    config->writeEntry("Settings Tab", d->advBox->currentIndex());
    config->writeEntry("AutoRotate", d->autoRotateCheck->isChecked());
    config->writeEntry("AutoAlbumDate", d->autoAlbumDateCheck->isChecked());
    config->writeEntry("AutoAlbumExt", d->autoAlbumExtCheck->isChecked());
    config->writeEntry("FixDateTime", d->fixDateTimeCheck->isChecked());
    config->writeEntry("SetPhotographerId", d->setPhotographerId->isChecked());
    config->writeEntry("SetCredits", d->setCredits->isChecked());
    config->writeEntry("ConvertJpeg", convertLosslessJpegFiles());
    config->writeEntry("LossLessFormat", d->losslessFormat->currentItem());
    config->writeEntry("ThumbnailSize", d->view->thumbnailSize().size());
    config->writeEntry("Splitter Sizes", d->splitter->sizes());
    config->writeEntry("FolderDateFormat", d->folderDateFormat->currentItem());
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

bool CameraUI::convertLosslessJpegFiles() const
{
    return d->convertJpegCheck->isChecked();
}

QString CameraUI::losslessFormat()
{
    return d->losslessFormat->currentText();
}

QString CameraUI::cameraTitle() const
{
    return d->cameraTitle;
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

    d->status->setText(i18n("Disconnecting from camera, please wait..."));
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
        d->closed = true;
        finishDialog();
    }

    return true;
}

void CameraUI::finishDialog()
{
    // Look if an item have been downloaded to computer during camera gui session.
    // If yes, update the lastAccess date property of camera in digiKam camera list.

    if (d->view->itemsDownloaded() > 0)
    {
        CameraList* clist = CameraList::instance();
        if (clist) 
            clist->changeCameraAccessTime(d->cameraTitle, QDateTime::QDateTime::currentDateTime());
    }

    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run scanlib
    // over the folders we used. Bug: 119201

    d->status->setText(i18n("Scanning for new files, please wait..."));
    ScanLib sLib;
    for (QStringList::iterator it = d->foldersToScan.begin();
         it != d->foldersToScan.end(); ++it)
    {
        //DDebug() << "Scanning " << (*it) << endl;
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
        // B.K.O #127614: The Focus need to be restored in custom prefix widget.
        //commenting this out again:  If we do not disable, no need to restore focus
        //d->renameCustomizer->restoreFocus();

        enableButton(User3, true);
        enableButton(User2, true);
        enableButton(User1, true);
        d->helpMenu->menu()->setItemEnabled(CAMERA_INFO_MENU_ID, true);

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
        // Settings tab is disabled in slotDownload, selectively when downloading
        // Fast dis/enabling would create the impression of flicker, e.g. when retrieving EXIF from camera
        //d->advBox->setEnabled(false);

        enableButton(User3, false);
        enableButton(User2, false);
        enableButton(User1, false);
        d->helpMenu->menu()->setItemEnabled(CAMERA_INFO_MENU_ID, false);
    }
}

void CameraUI::slotIncreaseThumbSize()
{
    ThumbnailSize thumbSize;

    switch(d->view->thumbnailSize().size())
    {
        case (ThumbnailSize::Small):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Medium);
            break;
        }
        case (ThumbnailSize::Medium):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Large);
            break;
        }
        case (ThumbnailSize::Large):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Huge);
            break;
        }
        case (ThumbnailSize::Huge):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Huge);
            break;
        }
        default:
            return;
    }

    if (thumbSize.size() == ThumbnailSize::Huge)
    {
        d->imageMenu->setItemEnabled(4, false);
    }
    d->imageMenu->setItemEnabled(5, true);

    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotDecreaseThumbSize()
{
    ThumbnailSize thumbSize;

    switch(d->view->thumbnailSize().size())
    {
        case (ThumbnailSize::Small):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Small);
            break;
        }
        case (ThumbnailSize::Medium):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Small);
            break;
        }
        case (ThumbnailSize::Large):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Medium);
            break;
        }
        case (ThumbnailSize::Huge):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Large);
            break;
        }
        default:
            return;
    }

    if (thumbSize.size() == ThumbnailSize::Small)
    {
        d->imageMenu->setItemEnabled(5, false);
    }
    d->imageMenu->setItemEnabled(4, true);    
    
    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotConnected(bool val)
{
    if (!val)
    {
      if (KMessageBox::warningYesNo(this,
                                    i18n("Failed to connect to the camera. "
                                         "Please make sure it is connected "
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

    d->progress->setProgress(0);
    d->progress->setTotalSteps(0);
    d->progress->show();

    d->cameraFolderList = folderList;
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

    if (fileList.empty())
        return;

    for (GPItemInfoList::const_iterator it = fileList.begin();
         it != fileList.end(); ++it)
    {
        GPItemInfo item = *it;

        if (item.mtime > (time_t)d->lastAccess.toTime_t() && item.downloaded == GPItemInfo::DownloadUnknow)
           item.downloaded = GPItemInfo::NewPicture;

        d->view->addItem(item);
        d->controller->getThumbnail(item.folder, item.name);
    }

    d->progress->setTotalSteps(d->progress->totalSteps() + fileList.count());
}

void CameraUI::slotThumbnail(const QString& folder, const QString& file,
                             const QImage& thumbnail)
{
    d->view->setThumbnail(folder, file, thumbnail);
    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotInformations()
{
    if (d->busy) 
        return;

    d->controller->getCameraInformations();
}

void CameraUI::slotCameraInformations(const QString& summary, const QString& manual, const QString& about)
{
    CameraInfoDialog *infoDlg = new CameraInfoDialog(this, summary, manual, about);
    infoDlg->show();
}

void CameraUI::slotErrorMsg(const QString& msg)
{
    KMessageBox::error(this, msg);    
}

void CameraUI::slotUpload()
{
    if (d->busy)
        return;

    QString fileformats;
    
    QStringList patternList = QStringList::split('\n', KImageIO::pattern(KImageIO::Reading));
    
    // All Pictures from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];
    
    // Add RAW file format to All Pictures" type mime and remplace current.
    allPictures.insert(allPictures.find("|"), QString(raw_file_extentions));
    patternList.remove(patternList[0]);
    patternList.prepend(allPictures);
    
    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unavailable(dcraw_0)(see file #121242 in B.K.O).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(raw_file_extentions)));
    
    fileformats = patternList.join("\n");

    DDebug () << "fileformats=" << fileformats << endl;   

    KURL::List urls = KFileDialog::getOpenURLs(AlbumManager::instance()->getLibraryPath(), 
                                               fileformats, this, i18n("Select Image to Upload"));
    if (!urls.isEmpty())
        slotUploadItems(urls);
}

void CameraUI::slotUploadItems(const KURL::List& urls)
{
    if (d->busy)
        return;

    if (urls.isEmpty())
        return;

    CameraFolderDialog dlg(this, d->view, d->cameraFolderList, d->controller->getCameraTitle(),
                           d->controller->getCameraPath());

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString cameraFolder = dlg.selectedFolderPath();

    for (KURL::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        QFileInfo fi((*it).path());
        if (!fi.exists()) continue;
        if (fi.isDir()) continue;

        QString ext  = QString(".") + fi.extension();
        QString name = fi.fileName();
        name.truncate(fi.fileName().length() - ext.length());

        bool ok;

        while (d->view->findItem(cameraFolder, name + ext)) 
        {
            QString msg(i18n("Camera Folder <b>%1</b> already contains item <b>%2</b><br>"
                             "Please enter a new file name (without extension):")
                             .arg(cameraFolder).arg(fi.fileName()));
#if KDE_IS_VERSION(3,2,0)
            name = KInputDialog::getText(i18n("File already exists"), msg, name, &ok, this);

#else
            name = KLineEditDlg::getText(i18n("File already exists"), msg, name, &ok, this);
#endif
            if (!ok) 
                return;
        }

        d->controller->upload(fi, name + ext, cameraFolder);
    }
}

void CameraUI::slotUploaded(const GPItemInfo& itemInfo)
{
    if (d->closed)
        return;

    d->view->addItem(itemInfo);
    d->controller->getThumbnail(itemInfo.folder, itemInfo.name);
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
    // -- Get the destination album from digiKam library ---------------

    AlbumManager* man = AlbumManager::instance();

    Album* album = man->currentAlbum();
    if (album && album->type() != Album::PHYSICAL)
        album = 0;

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "import the camera pictures into.</p>"));

    QString newDirName;
    IconItem* firstItem = d->view->firstItem();
    if (firstItem)
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(firstItem);
        
        QDateTime dateTime;
        dateTime.setTime_t(iconItem->itemInfo()->mtime);

        switch(d->folderDateFormat->currentItem())
        {
            case CameraUIPriv::TextDateFormat:
                newDirName = dateTime.date().toString(Qt::TextDate);
                break;
            case CameraUIPriv::LocalDateFormat:
                newDirName = dateTime.date().toString(Qt::LocalDate);
                break;
            default:        // IsoDateFormat
                newDirName = dateTime.date().toString(Qt::ISODate);
                break;
        }
    }

    album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header, newDirName,
                                           d->autoAlbumDateCheck->isChecked());

    if (!album)
        return;

    // -- Prepare downloading of camera items ------------------------

    KURL url;
    url.setPath(((PAlbum*)album)->folderPath());
    
    d->controller->downloadPrep();

    DownloadSettingsContainer downloadSettings;
    QString downloadName;
    time_t  mtime;
    int     total = 0;
    
    downloadSettings.autoRotate        = d->autoRotateCheck->isChecked();
    downloadSettings.fixDateTime       = d->fixDateTimeCheck->isChecked();
    downloadSettings.newDateTime       = d->dateTimeEdit->dateTime();
    downloadSettings.setPhotographerId = d->setPhotographerId->isChecked();
    downloadSettings.setCredits        = d->setCredits->isChecked();
    downloadSettings.convertJpeg       = convertLosslessJpegFiles();
    downloadSettings.losslessFormat    = losslessFormat();
    
    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        downloadSettings.author      = settings->getIptcAuthor();
        downloadSettings.authorTitle = settings->getIptcAuthorTitle();
        downloadSettings.credit      = settings->getIptcCredit();
        downloadSettings.source      = settings->getIptcSource();
        downloadSettings.copyright   = settings->getIptcCopyright();        
    }

    // -- Download camera items -------------------------------
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        if (onlySelected && !(item->isSelected()))
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        downloadSettings.folder      = iconItem->itemInfo()->folder;
        downloadSettings.file        = iconItem->itemInfo()->name;
        downloadName                 = iconItem->getDownloadName();
        mtime                        = iconItem->itemInfo()->mtime;

        // occurs if renameCustomizer->useDefault() and Download All is used.
        // If !useDefault, downloadName depends on selection,
        // so Download All is disabled! (see slotNewSelection)
        if (downloadName.isEmpty())
            downloadName = d->view->defaultDownloadName(iconItem);
        
        KURL u(url);
        QString errMsg;
        QDateTime dateTime;
        dateTime.setTime_t(mtime);
                    
        // Auto sub-albums creation based on file date.     

        if (d->autoAlbumDateCheck->isChecked())
        {
            QString dirName;

            switch(d->folderDateFormat->currentItem())
            {
                case CameraUIPriv::TextDateFormat:
                    dirName = dateTime.date().toString(Qt::TextDate);
                    break;
                case CameraUIPriv::LocalDateFormat:
                    dirName = dateTime.date().toString(Qt::LocalDate);
                    break;
                default:        // IsoDateFormat
                    dirName = dateTime.date().toString(Qt::ISODate);
                    break;
            }

            if (!createAutoAlbum(url, dirName, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(dirName);
        }

        // Auto sub-albums creation based on file extensions.

        if (d->autoAlbumExtCheck->isChecked())           
        {
            // We use the target file name to compute sub-albums name to take a care about 
            // convertion on the fly option.
            QFileInfo fi(downloadName);

            QString subAlbum = fi.extension(false).upper();
	        if (fi.extension(false).upper() == QString("JPEG") || 
                fi.extension(false).upper() == QString("JPE")) 
                subAlbum = QString("JPG");
            if (fi.extension(false).upper() == QString("TIFF")) 
                subAlbum = QString("TIF");
            if (fi.extension(false).upper() == QString("MPEG") || 
                fi.extension(false).upper() == QString("MPE") ||
                fi.extension(false).upper() == QString("MPO"))
                subAlbum = QString("MPG");

            if (!createAutoAlbum(u, subAlbum, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(subAlbum);
        }

        d->foldersToScan.append(u.path());
        u.addPath(downloadName.isEmpty() ? downloadSettings.file : downloadName);

        downloadSettings.dest = u.path();

        d->controller->download(downloadSettings);
        addFileExtension(QFileInfo(u.path()).extension(false));
        total++;
    }

    if (total <= 0)
        return;
    
    d->lastDestURL = url;
    d->progress->setProgress(0);
    d->progress->setTotalSteps(total);
    d->progress->show();

    // disable settings tab here instead of slotBusy:
    // Only needs to be disabled while downloading
    d->advBox->setEnabled(false);
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file, int status)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
    {
        iconItem->setDownloaded(status);
        d->view->ensureItemVisible(iconItem);

        //if (iconItem->isSelected())
          //  slotItemsSelected(iconItem, true);
    }
    
    if (status == GPItemInfo::DownloadedYes || status == GPItemInfo::DownloadFailed)
    {
        int curr = d->progress->progress();
        d->progress->setProgress(curr+1);
    }
}

void CameraUI::slotSkipped(const QString& folder, const QString& file)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
        d->view->ensureItemVisible(iconItem);

    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotToggleLock()
{
    int count = 0;
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            QString folder = iconItem->itemInfo()->folder;
            QString file   = iconItem->itemInfo()->name;
            int writePerm  = iconItem->itemInfo()->writePermissions;
            bool lock      = true;            

            // If item is currently locked, unlock it.
            if (writePerm == 0) 
                lock = false;

            d->controller->lockFile(folder, file, lock);
            count++;
        }
    }

    if (count > 0)
    {
        d->progress->setProgress(0);
        d->progress->setTotalSteps(count);
        d->progress->show();
    }
}

void CameraUI::slotLocked(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        CameraIconViewItem* iconItem = d->view->findItem(folder, file);
        if (iconItem)
        {
            iconItem->toggleLock();
            //if (iconItem->isSelected())
              //  slotItemsSelected(iconItem, true);
        }
    }

    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    QStringList lockedList;
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            if (iconItem->itemInfo()->writePermissions != 0)  // Item not locked ?
            {
                QString folder = iconItem->itemInfo()->folder;
                QString file   = iconItem->itemInfo()->name;
                folders.append(folder);
                files.append(file);
                deleteList.append(folder + QString("/") + file);
            }
            else
            {
                lockedList.append(iconItem->itemInfo()->name);
            }
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18n("The items listed below are locked by the camera (read-only). "
                             "These items will not be deleted. If you really want to delete these items, "
                             "please unlock them and try again."));
        KMessageBox::informationList(this, infoMsg, lockedList, i18n("Information"));
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

        d->progress->setProgress(0);
        d->progress->setTotalSteps(deleteList.count());
        d->progress->show();
    
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
    QStringList lockedList;
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->itemInfo()->writePermissions != 0)  // Item not locked ?
        {
            QString folder = iconItem->itemInfo()->folder;
            QString file   = iconItem->itemInfo()->name;
            folders.append(folder);
            files.append(file);
            deleteList.append(folder + QString("/") + file);
        }
        else
        {
            lockedList.append(iconItem->itemInfo()->name);
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18n("The items listed below are locked by camera (read-only). "
                             "These items will not be deleted. If you really want to delete these items, "
                             "please unlock them and try again."));        
        KMessageBox::informationList(this, infoMsg, lockedList, i18n("Information"));
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

        d->progress->setProgress(0);
        d->progress->setTotalSteps(deleteList.count());
        d->progress->show();
    
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotDeleted(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        d->view->removeItem(folder, file);
        // do this after removeItem, which will signal to slotItemsSelected, which checks for the list
        d->currentlyDeleting.remove(folder + file);
    }

    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotFileView(CameraIconViewItem* item)
{
    d->controller->openFile(item->itemInfo()->folder, item->itemInfo()->name);
}

void CameraUI::slotExifFromFile(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->view->findItem(folder, file);
    if (!item)
        return;

    d->rightSidebar->itemChanged(item->itemInfo(), folder + QString("/") + file, 
                                 QByteArray(), d->view, item);
}

void CameraUI::slotExifFromData(const QByteArray& exifData)
{
    CameraIconViewItem* item = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());

    if (!item)
        return;

    KURL url(item->itemInfo()->folder + '/' + item->itemInfo()->name);

    // Sometimes, GPhoto2 drivers return complete APP1 JFIF section. Exiv2 cannot 
    // decode (yet) exif metadata from APP1. We will find Exif header to get data at this place 
    // to please with Exiv2...

    DDebug() << "Size of Exif metadata from camera = " << exifData.size() << endl;
    char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
    
    if (!exifData.isEmpty())
    {
        int i = exifData.find(*exifHeader);
        if (i != -1)
        {
            DDebug() << "Exif header found at position " << i << endl;
            i = i + sizeof(exifHeader);
            QByteArray data(exifData.size()-i);
            memcpy(data.data(), exifData.data()+i, data.size());
            d->rightSidebar->itemChanged(item->itemInfo(), url, data, d->view, item);
            return;
        }
    }

    d->rightSidebar->itemChanged(item->itemInfo(), url, exifData, d->view, item);
}

void CameraUI::slotNewSelection(bool hasSelection)
{
    if (!d->renameCustomizer->useDefault())
    {
        // for customized names, the downloadNames depend on the selection.
        // So do not allow Download All if there is a selection!
        d->downloadMenu->setItemEnabled(0, hasSelection);
        d->downloadMenu->setItemEnabled(1, !hasSelection);
    }
    else
    {
        // if useDefault, the name can easily be computed without selection context,
        // so we can allow Download All.
        // This is the easiest default for new users
        d->downloadMenu->setItemEnabled(0, hasSelection);
        d->downloadMenu->setItemEnabled(1, true);
    }
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
            KURL url(item->itemInfo()->folder + '/' + item->itemInfo()->name);
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

bool CameraUI::createAutoAlbum(const KURL& parentURL, const QString& name,
                               const QDate& date, QString& errMsg)
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
    PAlbum* parent     = aman->findPAlbum(parentURL);
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

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events 
// to prevent any conflicts between dialog keys events and SpinBox keys events.

void CameraUI::keyPressEvent(QKeyEvent *e)
{
    if ( e->state() == 0 )
    {
        switch ( e->key() )
        {
        case Key_Escape:
            e->accept();
            reject();
        break;
        case Key_Enter:            
        case Key_Return:     
            e->ignore();              
        break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        // accept the dialog when Ctrl-Return is pressed
        if ( e->state() == ControlButton &&
            (e->key() == Key_Return || e->key() == Key_Enter) )
        {
            e->accept();
            accept();
        }
        else
        {
            e->ignore();
        }
    }
}

}  // namespace Digikam


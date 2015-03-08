/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancedlg.moc"

// Qt includes

#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QComboBox>
#include <QScrollArea>

// Libkdcraw includes

#include <libkdcraw/rexpanderbox.h>

// KDE includes

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kvbox.h>
#include <kconfig.h>

// Local includes

#include "config-digikam.h"
#include "setup.h"
#include "albumselectors.h"
#include "facescansettings.h"
#include "imagequalitysettings.h"
#include "metadatasynchronizer.h"

using namespace KDcrawIface;

namespace Digikam
{

class MaintenanceDlg::Private
{
public:

    enum Operation
    {
        Options = 0,
        NewItems,
        Thumbnails,
        FingerPrints,
        Duplicates,

#ifdef HAVE_KFACE
        FaceManagement,
#endif /* HAVE_KFACE */

        ImageQualitySorter,
        MetadataSync,

        Stretch
    };

public:

    Private() :
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerPrints(0),
        useMutiCoreCPU(0),
        qualityScanMode(0),
        metadataSetup(0),
        qualitySetup(0),
        syncDirection(0),
        hbox(0),
        vbox(0),
        vbox2(0),

#ifdef HAVE_KFACE
        hbox3(0),
        faceScannedHandling(0),
#endif /* HAVE_KFACE */

        similarity(0),
        expanderBox(0),
        albumSelectors(0)
    {
    }

    static const QString configGroupName;
    static const QString configUseMutiCoreCPU;
    static const QString configNewItems;
    static const QString configThumbnails;
    static const QString configScanThumbs;
    static const QString configFingerPrints;
    static const QString configScanFingerPrints;
    static const QString configDuplicates;
    static const QString configSimilarity;
    static const QString configFaceManagement;
    static const QString configFaceScannedHandling;
    static const QString configImageQualitySorter;
    static const QString configQualityScanMode;
    static const QString configMetadataSync;
    static const QString configSyncDirection;

    QLabel*              logo;
    QLabel*              title;
    QCheckBox*           scanThumbs;
    QCheckBox*           scanFingerPrints;
    QCheckBox*           useMutiCoreCPU;
    QComboBox*           qualityScanMode;
    QPushButton*         metadataSetup;
    QPushButton*         qualitySetup;
    QComboBox*           syncDirection;
    KHBox*               hbox;
    KVBox*               vbox;
    KVBox*               vbox2;

#ifdef HAVE_KFACE
    KHBox*               hbox3;
    QComboBox*           faceScannedHandling;
#endif /* HAVE_KFACE */

    KIntNumInput*        similarity;
    RExpanderBox*        expanderBox;
    AlbumSelectors*      albumSelectors;
};

const QString MaintenanceDlg::Private::configGroupName("MaintenanceDlg Settings");
const QString MaintenanceDlg::Private::configUseMutiCoreCPU("UseMutiCoreCPU");
const QString MaintenanceDlg::Private::configNewItems("NewItems");
const QString MaintenanceDlg::Private::configThumbnails("Thumbnails");
const QString MaintenanceDlg::Private::configScanThumbs("ScanThumbs");
const QString MaintenanceDlg::Private::configFingerPrints("FingerPrints");
const QString MaintenanceDlg::Private::configScanFingerPrints("ScanFingerPrints");
const QString MaintenanceDlg::Private::configDuplicates("Duplicates");
const QString MaintenanceDlg::Private::configSimilarity("Similarity");
const QString MaintenanceDlg::Private::configFaceManagement("FaceManagement");
const QString MaintenanceDlg::Private::configFaceScannedHandling("FaceScannedHandling");
const QString MaintenanceDlg::Private::configImageQualitySorter("ImageQualitySorter");
const QString MaintenanceDlg::Private::configQualityScanMode("QualityScanMode");
const QString MaintenanceDlg::Private::configMetadataSync("MetadataSync");
const QString MaintenanceDlg::Private::configSyncDirection("SyncDirection");

MaintenanceDlg::MaintenanceDlg(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    setHelp("digikam");
    setCaption(i18n("Maintenance"));
    setButtons(Ok | Help | Cancel);
    setDefaultButton(Cancel);

    QScrollArea* const main = new QScrollArea(this);
    QWidget* const page     = new QWidget(main->viewport());
    main->setWidget(page);
    main->setWidgetResizable(true);

    setMainWidget(main);

    QGridLayout* const grid = new QGridLayout(page);

    d->logo                 = new QLabel(page);
    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    d->title                = new QLabel(i18n("<qt><b>Select Maintenance Operations to Process</b></qt>"), page);
    d->expanderBox          = new RExpanderBox(page);

    // --------------------------------------------------------------------------------------

    KVBox* const options    = new KVBox;
    d->albumSelectors       = new AlbumSelectors(i18nc("@label", "Process items from:"), d->configGroupName, options);
    d->useMutiCoreCPU       = new QCheckBox(i18nc("@option:check", "Work on all processor cores"), options);
    d->expanderBox->insertItem(Private::Options, options, SmallIcon("configure"), i18n("Common Options"), "Options", true);

    // --------------------------------------------------------------------------------------

    d->expanderBox->insertItem(Private::NewItems, new QLabel(i18n("<qt>No option<br>"
                               "<i>Note: only Albums Collection are processed by this tool.</i></qt>")),
                               SmallIcon("view-refresh"), i18n("Scan for new items"), "NewItems", false);
    d->expanderBox->setCheckBoxVisible(Private::NewItems, true);

    // --------------------------------------------------------------------------------------

    d->scanThumbs        = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(Private::Thumbnails, d->scanThumbs, SmallIcon("view-process-all"),
                               i18n("Rebuild Thumbnails"), "Thumbnails", false);
    d->expanderBox->setCheckBoxVisible(Private::Thumbnails, true);

    // --------------------------------------------------------------------------------------

    d->scanFingerPrints  = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(Private::FingerPrints, d->scanFingerPrints, SmallIcon("run-build"),
                               i18n("Rebuild Finger-prints"), "Fingerprints", false);
    d->expanderBox->setCheckBoxVisible(Private::FingerPrints, true);

    // --------------------------------------------------------------------------------------

    d->hbox              = new KHBox;
    new QLabel(i18n("Similarity (in percents): "), d->hbox);
    QWidget* const space = new QWidget(d->hbox);
    d->hbox->setStretchFactor(space, 10);
    d->similarity        = new KIntNumInput(d->hbox);
    d->similarity->setValue(90);
    d->similarity->setRange(0, 100, 1);
    d->similarity->setSliderEnabled(false);
    d->expanderBox->insertItem(Private::Duplicates, d->hbox, SmallIcon("tools-wizard"),
                               i18n("Find Duplicates Items"), "Duplicates", false);
    d->expanderBox->setCheckBoxVisible(Private::Duplicates, true);

    // --------------------------------------------------------------------------------------

#ifdef HAVE_KFACE

    d->hbox3               = new KHBox;
    new QLabel(i18n("Faces data management: "), d->hbox3);
    QWidget* const space3  = new QWidget(d->hbox3);
    d->hbox3->setStretchFactor(space3, 10);
    d->faceScannedHandling = new QComboBox(d->hbox3);
    d->faceScannedHandling->addItem(i18n("Skip images already scanned"),          FaceScanSettings::Skip);
    d->faceScannedHandling->addItem(i18n("Scan again and merge results"),         FaceScanSettings::Merge);
    d->faceScannedHandling->addItem(i18n("Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->expanderBox->insertItem(Private::FaceManagement, d->hbox3, SmallIcon("edit-image-face-detect"),
                               i18n("Detect and recognize Faces (experimental)"), "FaceManagement", false);
    d->expanderBox->setCheckBoxVisible(Private::FaceManagement, true);

#endif /* HAVE_KFACE */

    // --------------------------------------------------------------------------------------

    d->vbox               = new KVBox;
    KHBox* const hbox11   = new KHBox(d->vbox);
    new QLabel(i18n("Scan Mode: "), hbox11);
    QWidget* const space7 = new QWidget(hbox11);
    hbox11->setStretchFactor(space7, 10);

    d->qualityScanMode    = new QComboBox(hbox11);
    d->qualityScanMode->addItem(i18n("Clean all and re-scan"),  ImageQualitySorter::AllItems);
    d->qualityScanMode->addItem(i18n("Scan non-assigned only"), ImageQualitySorter::NonAssignedItems);

    KHBox* const hbox12   = new KHBox(d->vbox);
    new QLabel(i18n("Check quality sorter setup panel for details: "), hbox12);
    QWidget* const space2 = new QWidget(hbox12);
    hbox12->setStretchFactor(space2, 10);
    d->qualitySetup       = new QPushButton(i18n("Settings..."), hbox12);
    d->expanderBox->insertItem(Private::ImageQualitySorter, d->vbox, SmallIcon("flag-green"),
                               i18n("Image Quality Sorter"), "ImageQualitySorter", false);
    d->expanderBox->setCheckBoxVisible(Private::ImageQualitySorter, true);

    // --------------------------------------------------------------------------------------

    d->vbox2              = new KVBox;
    KHBox* const hbox21   = new KHBox(d->vbox2);
    new QLabel(i18n("Sync Direction: "), hbox21);
    QWidget* const space5 = new QWidget(hbox21);
    hbox21->setStretchFactor(space5, 10);
    d->syncDirection      = new QComboBox(hbox21);
    d->syncDirection->addItem(i18n("From database to image metadata"), MetadataSynchronizer::WriteFromDatabaseToFile);
    d->syncDirection->addItem(i18n("From image metadata to database"), MetadataSynchronizer::ReadFromFileToDatabase);

    KHBox* const hbox22   = new KHBox(d->vbox2);
    new QLabel(i18n("Check metadata setup panel for details: "), hbox22);
    QWidget* const space6 = new QWidget(hbox22);
    hbox22->setStretchFactor(space6, 10);
    d->metadataSetup      = new QPushButton(i18n("Settings..."), hbox22);
    d->expanderBox->insertItem(Private::MetadataSync, d->vbox2, SmallIcon("run-build-file"),
                               i18n("Sync Metadata and Database"), "MetadataSync", false);
    d->expanderBox->setCheckBoxVisible(Private::MetadataSync, true);
    d->expanderBox->insertStretch(Private::Stretch);

    // --------------------------------------------------------------------------------------

    grid->addWidget(d->logo,                        0, 0, 1, 1);
    grid->addWidget(d->title,                       0, 1, 1, 1);
    grid->addWidget(d->expanderBox,                 5, 0, 3, 2);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(5, 10);

    // --------------------------------------------------------------------------------------

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));

    connect(d->expanderBox, SIGNAL(signalItemToggled(int,bool)),
            this, SLOT(slotItemToggled(int,bool)));

    connect(d->metadataSetup, SIGNAL(clicked()),
            this, SLOT(slotMetadataSetup()));

    connect(d->qualitySetup, SIGNAL(clicked()),
            this, SLOT(slotQualitySetup()));

    // --------------------------------------------------------------------------------------

    readSettings();
}

MaintenanceDlg::~MaintenanceDlg()
{
    delete d;
}

void MaintenanceDlg::slotOk()
{
    writeSettings();
}

MaintenanceSettings MaintenanceDlg::settings() const
{
    MaintenanceSettings prm;
    prm.wholeAlbums                         = d->albumSelectors->wholeAlbumsCollection();
    prm.wholeTags                           = d->albumSelectors->wholeTagsCollection();
    prm.albums                              = d->albumSelectors->selectedPAlbums();
    prm.tags                                = d->albumSelectors->selectedTAlbums();
    prm.useMutiCoreCPU                      = d->useMutiCoreCPU->isChecked();
    prm.newItems                            = d->expanderBox->isChecked(Private::NewItems);
    prm.thumbnails                          = d->expanderBox->isChecked(Private::Thumbnails);
    prm.scanThumbs                          = d->scanThumbs->isChecked();
    prm.fingerPrints                        = d->expanderBox->isChecked(Private::FingerPrints);
    prm.scanFingerPrints                    = d->scanFingerPrints->isChecked();
    prm.duplicates                          = d->expanderBox->isChecked(Private::Duplicates);
    prm.similarity                          = d->similarity->value();

#ifdef HAVE_KFACE
    prm.faceManagement                      = d->expanderBox->isChecked(Private::FaceManagement);
    prm.faceSettings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)d->faceScannedHandling->itemData(d->faceScannedHandling->currentIndex()).toInt();
    prm.faceSettings.albums                 = d->albumSelectors->selectedAlbums();
#endif /* HAVE_KFACE */

    prm.qualitySort                         = d->expanderBox->isChecked(Private::ImageQualitySorter);
    prm.qualityScanMode                     = d->qualityScanMode->itemData(d->qualityScanMode->currentIndex()).toInt();
    ImageQualitySettings imgq;
    imgq.readFromConfig();
    prm.quality                             = imgq;
    prm.metadataSync                        = d->expanderBox->isChecked(Private::MetadataSync);
    prm.syncDirection                       = d->syncDirection->itemData(d->syncDirection->currentIndex()).toInt();
    return prm;
}

void MaintenanceDlg::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->readSettings(group);
    d->albumSelectors->loadState();

    MaintenanceSettings prm;

    d->useMutiCoreCPU->setChecked(group.readEntry(d->configUseMutiCoreCPU,                               prm.useMutiCoreCPU));
    d->expanderBox->setChecked(Private::NewItems,           group.readEntry(d->configNewItems,           prm.newItems));
    d->expanderBox->setChecked(Private::Thumbnails,         group.readEntry(d->configThumbnails,         prm.thumbnails));
    d->scanThumbs->setChecked(group.readEntry(d->configScanThumbs,                                       prm.scanThumbs));
    d->expanderBox->setChecked(Private::FingerPrints,       group.readEntry(d->configFingerPrints,       prm.fingerPrints));
    d->scanFingerPrints->setChecked(group.readEntry(d->configScanFingerPrints,                           prm.scanFingerPrints));
    d->expanderBox->setChecked(Private::Duplicates,         group.readEntry(d->configDuplicates,         prm.duplicates));
    d->similarity->setValue(group.readEntry(d->configSimilarity,                                         prm.similarity));

#ifdef HAVE_KFACE
    d->expanderBox->setChecked(Private::FaceManagement,     group.readEntry(d->configFaceManagement,     prm.faceManagement));
    d->faceScannedHandling->setCurrentIndex(group.readEntry(d->configFaceScannedHandling,                (int)prm.faceSettings.alreadyScannedHandling));
#endif /* HAVE_KFACE */

    d->expanderBox->setChecked(Private::ImageQualitySorter, group.readEntry(d->configImageQualitySorter, prm.qualitySort));
    d->qualityScanMode->setCurrentIndex(group.readEntry(d->configQualityScanMode,                        prm.qualityScanMode));
    d->expanderBox->setChecked(Private::MetadataSync,       group.readEntry(d->configMetadataSync,       prm.metadataSync));
    d->syncDirection->setCurrentIndex(group.readEntry(d->configSyncDirection,                            prm.syncDirection));

    for (int i = Private::NewItems ; i < Private::Stretch ; ++i)
    {
        slotItemToggled(i, d->expanderBox->isChecked(i));
    }

    restoreDialogSize(group);
}

void MaintenanceDlg::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->writeSettings(group);
    d->albumSelectors->saveState();

    MaintenanceSettings prm   = settings();

    group.writeEntry(d->configUseMutiCoreCPU,      prm.useMutiCoreCPU);
    group.writeEntry(d->configNewItems,            prm.newItems);
    group.writeEntry(d->configThumbnails,          prm.thumbnails);
    group.writeEntry(d->configScanThumbs,          prm.scanThumbs);
    group.writeEntry(d->configFingerPrints,        prm.fingerPrints);
    group.writeEntry(d->configScanFingerPrints,    prm.scanFingerPrints);
    group.writeEntry(d->configDuplicates,          prm.duplicates);
    group.writeEntry(d->configSimilarity,          prm.similarity);

#ifdef HAVE_KFACE
    group.writeEntry(d->configFaceManagement,      prm.faceManagement);
    group.writeEntry(d->configFaceScannedHandling, (int)prm.faceSettings.alreadyScannedHandling);
#endif /* HAVE_KFACE */

    group.writeEntry(d->configImageQualitySorter,  prm.qualitySort);
    group.writeEntry(d->configQualityScanMode,     prm.qualityScanMode);
    group.writeEntry(d->configMetadataSync,        prm.metadataSync);
    group.writeEntry(d->configSyncDirection,       prm.syncDirection);

    saveDialogSize(group);
}

void MaintenanceDlg::slotItemToggled(int index, bool b)
{
    switch (index)
    {
        case Private::Thumbnails:
            d->scanThumbs->setEnabled(b);
            break;

        case Private::FingerPrints:
            d->scanFingerPrints->setEnabled(b);
            break;

        case Private::Duplicates:
            d->hbox->setEnabled(b);
            break;

#ifdef HAVE_KFACE
        case Private::FaceManagement:
            d->hbox3->setEnabled(b);
            break;
#endif /* HAVE_KFACE */

        case Private::ImageQualitySorter:
            d->vbox->setEnabled(b);
            break;

        case Private::MetadataSync:
            d->vbox2->setEnabled(b);
            break;

        default :  // NewItems
            break;
    }
}

void MaintenanceDlg::slotMetadataSetup()
{
    Setup::execSinglePage(this, Setup::MetadataPage);
}

void MaintenanceDlg::slotQualitySetup()
{
    Setup::execSinglePage(this, Setup::ImageQualityPage);
}

}  // namespace Digikam

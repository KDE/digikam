/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancedlg.h"

// Qt includes

#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QIcon>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "digikam_config.h"
#include "setup.h"
#include "albumselectors.h"
#include "facescansettings.h"
#include "imagequalitysettings.h"
#include "metadatasynchronizer.h"
#include "dxmlguiwindow.h"
#include "applicationsettings.h"
#include "drangebox.h"

namespace Digikam
{

class MaintenanceDlg::Private
{
public:

    enum Operation
    {
        Options = 0,
        NewItems,
        DbCleanup,
        Thumbnails,
        FingerPrints,
        Duplicates,
        FaceManagement,
        ImageQualitySorter,
        MetadataSync,
        Stretch
    };

public:

    Private() :
        buttons(0),
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerPrints(0),
        useMutiCoreCPU(0),
        cleanThumbsDb(0),
        cleanFacesDb(0),
        shrinkDatabases(0),
        qualityScanMode(0),
        metadataSetup(0),
        qualitySetup(0),
        syncDirection(0),
        similarityRangeBox(0),
        dupeRestrictionBox(0),
        vbox(0),
        vbox2(0),
        vbox3(0),
        duplicatesBox(0),
        hbox3(0),
        similarityRange(0),
        faceScannedHandling(0),
        searchResultRestriction(0),
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
    static const QString configMinSimilarity;
    static const QString configMaxSimilarity;
    static const QString configDuplicatesRestriction;
    static const QString configFaceManagement;
    static const QString configFaceScannedHandling;
    static const QString configImageQualitySorter;
    static const QString configQualityScanMode;
    static const QString configMetadataSync;
    static const QString configCleanupDatabase;
    static const QString configCleanupThumbDatabase;
    static const QString configCleanupFacesDatabase;
    static const QString configShrinkDatabases;
    static const QString configSyncDirection;

    QDialogButtonBox*    buttons;
    QLabel*              logo;
    QLabel*              title;
    QCheckBox*           scanThumbs;
    QCheckBox*           scanFingerPrints;
    QCheckBox*           useMutiCoreCPU;
    QCheckBox*           cleanThumbsDb;
    QCheckBox*           cleanFacesDb;
    QCheckBox*           shrinkDatabases;
    QComboBox*           qualityScanMode;
    QPushButton*         metadataSetup;
    QPushButton*         qualitySetup;
    QComboBox*           syncDirection;
    DHBox*               similarityRangeBox;
    DHBox*               dupeRestrictionBox;
    DVBox*               vbox;
    DVBox*               vbox2;
    DVBox*               vbox3;
    DVBox*               duplicatesBox;
    DHBox*               hbox3;
    DIntRangeBox*        similarityRange;
    QComboBox*           faceScannedHandling;
    QComboBox*           searchResultRestriction;
    DExpanderBox*        expanderBox;
    AlbumSelectors*      albumSelectors;
};

const QString MaintenanceDlg::Private::configGroupName(QLatin1String("MaintenanceDlg Settings"));
const QString MaintenanceDlg::Private::configUseMutiCoreCPU(QLatin1String("UseMutiCoreCPU"));
const QString MaintenanceDlg::Private::configNewItems(QLatin1String("NewItems"));
const QString MaintenanceDlg::Private::configThumbnails(QLatin1String("Thumbnails"));
const QString MaintenanceDlg::Private::configScanThumbs(QLatin1String("ScanThumbs"));
const QString MaintenanceDlg::Private::configFingerPrints(QLatin1String("FingerPrints"));
const QString MaintenanceDlg::Private::configScanFingerPrints(QLatin1String("ScanFingerPrints"));
const QString MaintenanceDlg::Private::configDuplicates(QLatin1String("Duplicates"));
const QString MaintenanceDlg::Private::configMinSimilarity(QLatin1String("minSimilarity"));
const QString MaintenanceDlg::Private::configMaxSimilarity(QLatin1String("maxSimilarity"));
const QString MaintenanceDlg::Private::configDuplicatesRestriction(QLatin1String("duplicatesRestriction"));
const QString MaintenanceDlg::Private::configFaceManagement(QLatin1String("FaceManagement"));
const QString MaintenanceDlg::Private::configFaceScannedHandling(QLatin1String("FaceScannedHandling"));
const QString MaintenanceDlg::Private::configImageQualitySorter(QLatin1String("ImageQualitySorter"));
const QString MaintenanceDlg::Private::configQualityScanMode(QLatin1String("QualityScanMode"));
const QString MaintenanceDlg::Private::configMetadataSync(QLatin1String("MetadataSync"));
const QString MaintenanceDlg::Private::configSyncDirection(QLatin1String("SyncDirection"));
const QString MaintenanceDlg::Private::configCleanupDatabase(QLatin1String("CleanupDatabase"));
const QString MaintenanceDlg::Private::configCleanupThumbDatabase(QLatin1String("CleanupThumbDatabase"));
const QString MaintenanceDlg::Private::configCleanupFacesDatabase(QLatin1String("CleanupFacesDatabase"));
const QString MaintenanceDlg::Private::configShrinkDatabases(QLatin1String("ShrinkDatabases"));

MaintenanceDlg::MaintenanceDlg(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Maintenance"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    QScrollArea* const main    = new QScrollArea(this);
    QWidget* const page        = new QWidget(main->viewport());
    main->setWidget(page);
    main->setWidgetResizable(true);

    QGridLayout* const grid    = new QGridLayout(page);

    d->logo                    = new QLabel(page);
    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));
    d->title                   = new QLabel(i18n("<qt><b>Select Maintenance Operations to Process</b></qt>"), page);
    d->expanderBox             = new DExpanderBox(page);

    // --------------------------------------------------------------------------------------

    DVBox* const options       = new DVBox;
    d->albumSelectors          = new AlbumSelectors(i18nc("@label", "Process items from:"), d->configGroupName, options);
    d->useMutiCoreCPU          = new QCheckBox(i18nc("@option:check", "Work on all processor cores (when it possible)"), options);
    d->expanderBox->insertItem(Private::Options, options, QIcon::fromTheme(QLatin1String("configure")), i18n("Common Options"), QLatin1String("Options"), true);

    // --------------------------------------------------------------------------------------

    d->expanderBox->insertItem(Private::NewItems, new QLabel(i18n("<qt>No option<br>"
                               "<i>Note: only Albums Collection are processed by this tool.</i></qt>")),
                               QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Scan for new items"), QLatin1String("NewItems"), false);
    d->expanderBox->setCheckBoxVisible(Private::NewItems, true);

    // --------------------------------------------------------------------------------------

    d->vbox3                   = new DVBox;
    new QLabel(i18n("<qt><i>Note: If activated, the Core DB is always cleaned. You can select additional databases for cleaning.<br/>"
                    " If you select one of the options below, the process may take much time and can freeze digiKam temporarily<br/>"
                    " in order to make sure that no database corruption occurs.</i></qt>"), d->vbox3);
    d->cleanThumbsDb           = new QCheckBox(i18n("Also clean up the thumbnail database."), d->vbox3);
    d->cleanFacesDb            = new QCheckBox(i18n("Also clean up the faces database."), d->vbox3);
    d->shrinkDatabases         = new QCheckBox(i18n("Also shrink all databases if possible."), d->vbox3);
    d->shrinkDatabases->setToolTip(i18n("This option leads to the vacuuming (shrinking) of the databases."
                                        " Vacuuming is supported both for SQLite and MySQL."));
    d->expanderBox->insertItem(Private::DbCleanup, d->vbox3,
                               QIcon::fromTheme(QLatin1String("run-build")),
                               i18n("Perform database cleaning"), QLatin1String("DbCleanup"), false);
    d->expanderBox->setCheckBoxVisible(Private::DbCleanup, true);

    // --------------------------------------------------------------------------------------

    d->scanThumbs              = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(Private::Thumbnails, d->scanThumbs, QIcon::fromTheme(QLatin1String("view-process-all")),
                               i18n("Rebuild Thumbnails"), QLatin1String("Thumbnails"), false);
    d->expanderBox->setCheckBoxVisible(Private::Thumbnails, true);

    // --------------------------------------------------------------------------------------

    d->scanFingerPrints        = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(Private::FingerPrints, d->scanFingerPrints, QIcon::fromTheme(QLatin1String("run-build")),
                               i18n("Rebuild Finger-prints"), QLatin1String("Fingerprints"), false);
    d->expanderBox->setCheckBoxVisible(Private::FingerPrints, true);

    // --------------------------------------------------------------------------------------

    const ApplicationSettings * settings = ApplicationSettings::instance();

    d->duplicatesBox           = new DVBox;
    d->similarityRangeBox      = new DHBox(d->duplicatesBox);
    new QLabel(i18n("Similarity range (in percents): "), d->similarityRangeBox);
    QWidget* const space       = new QWidget(d->similarityRangeBox);
    d->similarityRangeBox->setStretchFactor(space, 10);

    d->similarityRange = new DIntRangeBox(d->similarityRangeBox);
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (settings)
    {
        d->similarityRange->setRange(settings->getMinimumSimilarityBound(), 100);
        d->similarityRange->setInterval(settings->getDuplicatesSearchLastMinSimilarity(),
                                        settings->getDuplicatesSearchLastMaxSimilarity());
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(90, 100);
    }

    d->dupeRestrictionBox      = new DHBox(d->duplicatesBox);
    new QLabel(i18n("Restriction on duplicates:"), d->dupeRestrictionBox);
    QWidget* const space4      = new QWidget(d->dupeRestrictionBox);
    d->dupeRestrictionBox->setStretchFactor(space4, 10);
    d->searchResultRestriction = new QComboBox(d->dupeRestrictionBox);
    d->searchResultRestriction->addItem(i18n("No restriction"),                       HaarIface::DuplicatesSearchRestrictions::None);
    d->searchResultRestriction->addItem(i18n("Restrict to album of reference image"), HaarIface::DuplicatesSearchRestrictions::SameAlbum);
    d->searchResultRestriction->addItem(i18n("Exclude album of reference image"),     HaarIface::DuplicatesSearchRestrictions::DifferentAlbum);

    // Load the last choice from application settings.
    HaarIface::DuplicatesSearchRestrictions restrictions = HaarIface::DuplicatesSearchRestrictions::None;

    if (settings)
    {
        restrictions = (HaarIface::DuplicatesSearchRestrictions) settings->getDuplicatesSearchRestrictions();
    }

    d->searchResultRestriction->setCurrentIndex(d->searchResultRestriction->findData(restrictions));

    d->expanderBox->insertItem(Private::Duplicates, d->duplicatesBox, QIcon::fromTheme(QLatin1String("tools-wizard")),
                               i18n("Find Duplicate Items"), QLatin1String("Duplicates"), false);
    d->expanderBox->setCheckBoxVisible(Private::Duplicates, true);

    // --------------------------------------------------------------------------------------

    d->hbox3               = new DHBox;
    new QLabel(i18n("Faces data management: "), d->hbox3);
    QWidget* const space3  = new QWidget(d->hbox3);
    d->hbox3->setStretchFactor(space3, 10);
    d->faceScannedHandling = new QComboBox(d->hbox3);
    d->faceScannedHandling->addItem(i18n("Skip images already scanned"),          FaceScanSettings::Skip);
    d->faceScannedHandling->addItem(i18n("Scan again and merge results"),         FaceScanSettings::Merge);
    d->faceScannedHandling->addItem(i18n("Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->expanderBox->insertItem(Private::FaceManagement, d->hbox3, QIcon::fromTheme(QLatin1String("edit-image-face-detect")),
                               i18n("Detect and recognize Faces (experimental)"), QLatin1String("FaceManagement"), false);
    d->expanderBox->setCheckBoxVisible(Private::FaceManagement, true);

    // --------------------------------------------------------------------------------------

    d->vbox               = new DVBox;
    DHBox* const hbox11   = new DHBox(d->vbox);
    new QLabel(i18n("Scan Mode: "), hbox11);
    QWidget* const space7 = new QWidget(hbox11);
    hbox11->setStretchFactor(space7, 10);

    d->qualityScanMode    = new QComboBox(hbox11);
    d->qualityScanMode->addItem(i18n("Clean all and re-scan"),  ImageQualitySorter::AllItems);
    d->qualityScanMode->addItem(i18n("Scan non-assigned only"), ImageQualitySorter::NonAssignedItems);

    DHBox* const hbox12   = new DHBox(d->vbox);
    new QLabel(i18n("Check quality sorter setup panel for details: "), hbox12);
    QWidget* const space2 = new QWidget(hbox12);
    hbox12->setStretchFactor(space2, 10);
    d->qualitySetup       = new QPushButton(i18n("Settings..."), hbox12);
    d->expanderBox->insertItem(Private::ImageQualitySorter, d->vbox, QIcon::fromTheme(QLatin1String("flag-green")),
                               i18n("Image Quality Sorter"), QLatin1String("ImageQualitySorter"), false);
    d->expanderBox->setCheckBoxVisible(Private::ImageQualitySorter, true);

    // --------------------------------------------------------------------------------------

    d->vbox2              = new DVBox;
    DHBox* const hbox21   = new DHBox(d->vbox2);
    new QLabel(i18n("Sync Direction: "), hbox21);
    QWidget* const space5 = new QWidget(hbox21);
    hbox21->setStretchFactor(space5, 10);
    d->syncDirection      = new QComboBox(hbox21);
    d->syncDirection->addItem(i18n("From database to image metadata"), MetadataSynchronizer::WriteFromDatabaseToFile);
    d->syncDirection->addItem(i18n("From image metadata to database"), MetadataSynchronizer::ReadFromFileToDatabase);

    DHBox* const hbox22   = new DHBox(d->vbox2);
    new QLabel(i18n("Check metadata setup panel for details: "), hbox22);
    QWidget* const space6 = new QWidget(hbox22);
    hbox22->setStretchFactor(space6, 10);
    d->metadataSetup      = new QPushButton(i18n("Settings..."), hbox22);
    d->expanderBox->insertItem(Private::MetadataSync, d->vbox2, QIcon::fromTheme(QLatin1String("run-build-file")),
                               i18n("Sync Metadata and Database"), QLatin1String("MetadataSync"), false);
    d->expanderBox->setCheckBoxVisible(Private::MetadataSync, true);

    d->expanderBox->insertStretch(Private::Stretch);

    // --------------------------------------------------------------------------------------

    grid->addWidget(d->logo,                        0, 0, 1, 1);
    grid->addWidget(d->title,                       0, 1, 1, 1);
    grid->addWidget(d->expanderBox,                 5, 0, 3, 2);
    grid->setSpacing(style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setContentsMargins(QMargins());
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(5, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(main);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // --------------------------------------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

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
    accept();
}

MaintenanceSettings MaintenanceDlg::settings() const
{
    MaintenanceSettings prm;
    prm.wholeAlbums                         = d->albumSelectors->wholeAlbumsChecked();
    prm.wholeTags                           = d->albumSelectors->wholeTagsChecked();
    prm.albums                              = d->albumSelectors->selectedAlbums();
    prm.tags                                = d->albumSelectors->selectedTags();
    prm.useMutiCoreCPU                      = d->useMutiCoreCPU->isChecked();
    prm.newItems                            = d->expanderBox->isChecked(Private::NewItems);
    prm.databaseCleanup                     = d->expanderBox->isChecked(Private::DbCleanup);
    prm.cleanThumbDb                        = d->cleanThumbsDb->isChecked();
    prm.cleanFacesDb                        = d->cleanFacesDb->isChecked();
    prm.shrinkDatabases                     = d->shrinkDatabases->isChecked();
    prm.thumbnails                          = d->expanderBox->isChecked(Private::Thumbnails);
    prm.scanThumbs                          = d->scanThumbs->isChecked();
    prm.fingerPrints                        = d->expanderBox->isChecked(Private::FingerPrints);
    prm.scanFingerPrints                    = d->scanFingerPrints->isChecked();
    prm.duplicates                          = d->expanderBox->isChecked(Private::Duplicates);
    prm.minSimilarity                       = d->similarityRange->minValue();
    prm.maxSimilarity                       = d->similarityRange->maxValue();
    prm.duplicatesRestriction               = (HaarIface::DuplicatesSearchRestrictions)d->searchResultRestriction->itemData(d->searchResultRestriction->currentIndex()).toInt();
    prm.faceManagement                      = d->expanderBox->isChecked(Private::FaceManagement);
    prm.faceSettings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)d->faceScannedHandling->itemData(d->faceScannedHandling->currentIndex()).toInt();
    prm.faceSettings.albums                 = d->albumSelectors->selectedAlbums();
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->readSettings(group);
    d->albumSelectors->loadState();

    MaintenanceSettings prm;

    d->useMutiCoreCPU->setChecked(group.readEntry(d->configUseMutiCoreCPU,                               prm.useMutiCoreCPU));
    d->expanderBox->setChecked(Private::NewItems,           group.readEntry(d->configNewItems,           prm.newItems));

    d->expanderBox->setChecked(Private::DbCleanup,          group.readEntry(d->configCleanupDatabase,       prm.databaseCleanup));
    d->cleanThumbsDb->setChecked(group.readEntry(d->configCleanupThumbDatabase,                             prm.cleanThumbDb));
    d->cleanFacesDb->setChecked(group.readEntry(d->configCleanupFacesDatabase,                              prm.cleanFacesDb));
    d->shrinkDatabases->setChecked(group.readEntry(d->configShrinkDatabases,                                prm.shrinkDatabases));

    d->expanderBox->setChecked(Private::Thumbnails,         group.readEntry(d->configThumbnails,            prm.thumbnails));
    d->scanThumbs->setChecked(group.readEntry(d->configScanThumbs,                                          prm.scanThumbs));

    d->expanderBox->setChecked(Private::FingerPrints,       group.readEntry(d->configFingerPrints,          prm.fingerPrints));
    d->scanFingerPrints->setChecked(group.readEntry(d->configScanFingerPrints,                              prm.scanFingerPrints));

    d->expanderBox->setChecked(Private::Duplicates,         group.readEntry(d->configDuplicates,            prm.duplicates));
    d->similarityRange->setInterval(group.readEntry(d->configMinSimilarity,                                 prm.minSimilarity),
                                    group.readEntry(d->configMaxSimilarity,                                 prm.maxSimilarity));
    int restrictions = d->searchResultRestriction->findData(group.readEntry(d->configDuplicatesRestriction, (int)prm.duplicatesRestriction));
    d->searchResultRestriction->setCurrentIndex(restrictions);

    d->expanderBox->setChecked(Private::FaceManagement,     group.readEntry(d->configFaceManagement,        prm.faceManagement));
    d->faceScannedHandling->setCurrentIndex(group.readEntry(d->configFaceScannedHandling,                   (int)prm.faceSettings.alreadyScannedHandling));

    d->expanderBox->setChecked(Private::ImageQualitySorter, group.readEntry(d->configImageQualitySorter,    prm.qualitySort));
    d->qualityScanMode->setCurrentIndex(group.readEntry(d->configQualityScanMode,                           prm.qualityScanMode));

    d->expanderBox->setChecked(Private::MetadataSync,       group.readEntry(d->configMetadataSync,          prm.metadataSync));
    d->syncDirection->setCurrentIndex(group.readEntry(d->configSyncDirection,                               prm.syncDirection));

    for (int i = Private::NewItems ; i < Private::Stretch ; ++i)
    {
        slotItemToggled(i, d->expanderBox->isChecked(i));
    }

    winId();
    windowHandle()->resize(800, 600);
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

void MaintenanceDlg::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->writeSettings(group);
    d->albumSelectors->saveState();

    MaintenanceSettings prm   = settings();

    group.writeEntry(d->configUseMutiCoreCPU,        prm.useMutiCoreCPU);
    group.writeEntry(d->configNewItems,              prm.newItems);
    group.writeEntry(d->configCleanupDatabase,       prm.databaseCleanup);
    group.writeEntry(d->configCleanupThumbDatabase,  prm.cleanThumbDb);
    group.writeEntry(d->configCleanupFacesDatabase,  prm.cleanFacesDb);
    group.writeEntry(d->configShrinkDatabases,       prm.shrinkDatabases);
    group.writeEntry(d->configThumbnails,            prm.thumbnails);
    group.writeEntry(d->configScanThumbs,            prm.scanThumbs);
    group.writeEntry(d->configFingerPrints,          prm.fingerPrints);
    group.writeEntry(d->configScanFingerPrints,      prm.scanFingerPrints);
    group.writeEntry(d->configDuplicates,            prm.duplicates);
    group.writeEntry(d->configMinSimilarity,         prm.minSimilarity);
    group.writeEntry(d->configMaxSimilarity,         prm.maxSimilarity);
    group.writeEntry(d->configDuplicatesRestriction, (int)prm.duplicatesRestriction);
    group.writeEntry(d->configFaceManagement,        prm.faceManagement);
    group.writeEntry(d->configFaceScannedHandling,   (int)prm.faceSettings.alreadyScannedHandling);
    group.writeEntry(d->configImageQualitySorter,    prm.qualitySort);
    group.writeEntry(d->configQualityScanMode,       prm.qualityScanMode);
    group.writeEntry(d->configMetadataSync,          prm.metadataSync);
    group.writeEntry(d->configSyncDirection,         prm.syncDirection);

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
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
            d->duplicatesBox->setEnabled(b);
            break;

        case Private::FaceManagement:
            d->hbox3->setEnabled(b);
            break;

        case Private::ImageQualitySorter:
            d->vbox->setEnabled(b);
            break;

        case Private::MetadataSync:
            d->vbox2->setEnabled(b);
            break;

        case Private::DbCleanup:
            d->vbox3->setEnabled(b);
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

void MaintenanceDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam

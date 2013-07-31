/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Libkdcraw includes

#include <libkdcraw/rexpanderbox.h>

// KDE includes

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kvbox.h>
#include <kseparator.h>
#include <kconfig.h>

// Local includes

#include "setup.h"
#include "facescansettings.h"
#include "metadatasynchronizer.h"

using namespace KDcrawIface;

namespace Digikam
{

class MaintenanceDlg::Private
{
public:

    enum Operation
    {
        NewItems = 0,
        Thumbnails,
        FingerPrints,
        Duplicates,
        FaceManagement,
        MetadataSync,
        Stretch
    };

public:

    Private() :
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerPrints(0),
        faceScannedHandling(0),
        metadataSetup(0),
        syncDirection(0),
        hbox(0),
        vbox(0),
        hbox3(0),
        similarity(0),
        expanderBox(0)
    {
    }

    static const QString configGroupName;
    static const QString configNewItems;
    static const QString configThumbnails;
    static const QString configScanThumbs;
    static const QString configFingerPrints;
    static const QString configScanFingerPrints;
    static const QString configDuplicates;
    static const QString configSimilarity;
    static const QString configFaceManagement;
    static const QString configFaceScannedHandling;
    static const QString configMetadataSync;
    static const QString configSyncDirection;

    QLabel*              logo;
    QLabel*              title;
    QCheckBox*           scanThumbs;
    QCheckBox*           scanFingerPrints;
    QComboBox*           faceScannedHandling;
    QPushButton*         metadataSetup;
    QComboBox*           syncDirection;
    KHBox*               hbox;
    KVBox*               vbox;
    KHBox*               hbox3;
    KIntNumInput*        similarity;
    RExpanderBox*        expanderBox;
};

const QString MaintenanceDlg::Private::configGroupName("MaintenanceDlg Settings");
const QString MaintenanceDlg::Private::configNewItems("NewItems");
const QString MaintenanceDlg::Private::configThumbnails("Thumbnails");
const QString MaintenanceDlg::Private::configScanThumbs("ScanThumbs");
const QString MaintenanceDlg::Private::configFingerPrints("FingerPrints");
const QString MaintenanceDlg::Private::configScanFingerPrints("ScanFingerPrints");
const QString MaintenanceDlg::Private::configDuplicates("Duplicates");
const QString MaintenanceDlg::Private::configSimilarity("Similarity");
const QString MaintenanceDlg::Private::configFaceManagement("FaceManagement");
const QString MaintenanceDlg::Private::configFaceScannedHandling("FaceScannedHandling");
const QString MaintenanceDlg::Private::configMetadataSync("MetadataSync");
const QString MaintenanceDlg::Private::configSyncDirection("SyncDirection");

MaintenanceDlg::MaintenanceDlg(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    setHelp("digikam");
    setCaption(i18n("Maintenance"));
    setButtons(Ok | Help | Cancel);
    setDefaultButton(Cancel);

    QWidget* const page     = new QWidget(this);
    setMainWidget(page);

    QGridLayout* const grid = new QGridLayout(page);

    d->logo                 = new QLabel(page);
    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->title                = new QLabel(i18n("<qt><b>Select Maintenance Operations to Process</b></qt>"), page);
    d->expanderBox          = new RExpanderBox(page);
    KSeparator* const line  = new KSeparator(Qt::Horizontal);

    // --------------------------------------------------------------------------------------

    d->expanderBox->insertItem(Private::NewItems, new QLabel(i18n("<qt><i>no option</i></qt>")),
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

    d->hbox3               = new KHBox;
    new QLabel(i18n("Faces data management: "), d->hbox3);
    QWidget* const space3  = new QWidget(d->hbox3);
    d->hbox3->setStretchFactor(space3, 10);
    d->faceScannedHandling = new QComboBox(d->hbox3);
    d->faceScannedHandling->addItem(i18n("Skip images already scanned"),          FaceScanSettings::Skip);
    d->faceScannedHandling->addItem(i18n("Scan again and merge results"),         FaceScanSettings::Merge);
    d->faceScannedHandling->addItem(i18n("Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->expanderBox->insertItem(Private::FaceManagement, d->hbox3, SmallIcon("edit-image-face-detect"),
                               i18n("Detect and recognize Faces"), "FaceManagement", false);
    d->expanderBox->setCheckBoxVisible(Private::FaceManagement, true);

    // --------------------------------------------------------------------------------------

    d->vbox              = new KVBox;
    KHBox* const hbox21  = new KHBox(d->vbox);
    new QLabel(i18n("Sync Direction: "), hbox21);
    QWidget* const space4  = new QWidget(hbox21);
    hbox21->setStretchFactor(space4, 10);
    d->syncDirection = new QComboBox(hbox21);
    d->syncDirection->addItem(i18n("From database to image metadata"), MetadataSynchronizer::WriteFromDatabaseToFile);
    d->syncDirection->addItem(i18n("From image metadata to database"), MetadataSynchronizer::ReadFromFileToDatabase);

    KHBox* const hbox22  = new KHBox(d->vbox);
    new QLabel(i18n("Check metadata setup panel for details: "), hbox22);
    QWidget* const space2 = new QWidget(hbox22);
    hbox22->setStretchFactor(space2, 10);
    d->metadataSetup      = new QPushButton(i18n("Settings..."), hbox22);
    d->expanderBox->insertItem(Private::MetadataSync, d->vbox, SmallIcon("run-build-file"),
                               i18n("Sync Metadata and Database"), "MetadataSync", false);
    d->expanderBox->setCheckBoxVisible(Private::MetadataSync, true);
    d->expanderBox->insertStretch(Private::Stretch);

    // --------------------------------------------------------------------------------------

    grid->addWidget(d->logo,        0, 0, 1, 1);
    grid->addWidget(d->title,       0, 1, 1, 1);
    grid->addWidget(line,           1, 1, 1, 1);
    grid->addWidget(d->expanderBox, 2, 0, 3, 2);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);

    // --------------------------------------------------------------------------------------

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));

    connect(d->expanderBox, SIGNAL(signalItemToggled(int,bool)),
            this, SLOT(slotItemToggled(int,bool)));

    connect(d->metadataSetup, SIGNAL(clicked()),
            this, SLOT(slotMetadataSetup()));

    setMinimumSize(500, 350);
    adjustSize();
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
    prm.newItems                            = d->expanderBox->isChecked(Private::NewItems);
    prm.thumbnails                          = d->expanderBox->isChecked(Private::Thumbnails);
    prm.scanThumbs                          = d->scanThumbs->isChecked();
    prm.fingerPrints                        = d->expanderBox->isChecked(Private::FingerPrints);
    prm.scanFingerPrints                    = d->scanFingerPrints->isChecked();
    prm.duplicates                          = d->expanderBox->isChecked(Private::Duplicates);
    prm.similarity                          = d->similarity->value();
    prm.faceManagement                      = d->expanderBox->isChecked(Private::FaceManagement);
    prm.faceSettings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)d->faceScannedHandling->currentIndex();
    prm.metadataSync                        = d->expanderBox->isChecked(Private::MetadataSync);
    prm.syncDirection                       = d->syncDirection->currentIndex();
    return prm;
}

void MaintenanceDlg::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->readSettings(group);

    MaintenanceSettings prm;

    d->expanderBox->setChecked(Private::NewItems,       group.readEntry(d->configNewItems,       prm.newItems));
    d->expanderBox->setChecked(Private::Thumbnails,     group.readEntry(d->configThumbnails,     prm.thumbnails));
    d->scanThumbs->setChecked(group.readEntry(d->configScanThumbs,                               prm.scanThumbs));
    d->expanderBox->setChecked(Private::FingerPrints,   group.readEntry(d->configFingerPrints,   prm.fingerPrints));
    d->scanFingerPrints->setChecked(group.readEntry(d->configScanFingerPrints,                   prm.scanFingerPrints));
    d->expanderBox->setChecked(Private::Duplicates,     group.readEntry(d->configDuplicates,     prm.duplicates));
    d->similarity->setValue(group.readEntry(d->configSimilarity,                                 prm.similarity));
    d->expanderBox->setChecked(Private::FaceManagement, group.readEntry(d->configFaceManagement, prm.faceManagement));
    d->faceScannedHandling->setCurrentIndex(group.readEntry(d->configFaceScannedHandling,        (int)prm.faceSettings.alreadyScannedHandling));
    d->expanderBox->setChecked(Private::MetadataSync,   group.readEntry(d->configMetadataSync,   prm.metadataSync));
    d->syncDirection->setCurrentIndex(group.readEntry(d->configSyncDirection,                    prm.syncDirection));

    for (int i = Private::NewItems ; i < Private::Stretch ; ++i)
    {
        slotItemToggled(i, d->expanderBox->isChecked(i));
    }
}

void MaintenanceDlg::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->writeSettings(group);

    MaintenanceSettings prm   = settings();

    group.writeEntry(d->configNewItems,            prm.newItems);
    group.writeEntry(d->configThumbnails,          prm.thumbnails);
    group.writeEntry(d->configScanThumbs,          prm.scanThumbs);
    group.writeEntry(d->configFingerPrints,        prm.fingerPrints);
    group.writeEntry(d->configScanFingerPrints,    prm.scanFingerPrints);
    group.writeEntry(d->configDuplicates,          prm.duplicates);
    group.writeEntry(d->configSimilarity,          prm.similarity);
    group.writeEntry(d->configFaceManagement,      prm.faceManagement);
    group.writeEntry(d->configFaceScannedHandling, (int)prm.faceSettings.alreadyScannedHandling);
    group.writeEntry(d->configMetadataSync,        prm.metadataSync);
    group.writeEntry(d->configSyncDirection,       prm.syncDirection);
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

        case Private::FaceManagement:
            d->hbox3->setEnabled(b);
            break;

        case Private::MetadataSync:
            d->vbox->setEnabled(b);
            break;

        default :  // NewItems
            break;
    }
}

void MaintenanceDlg::slotMetadataSetup()
{
    Setup::execSinglePage(this, Setup::MetadataPage);
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <khbox.h>
#include <kseparator.h>
#include <kconfig.h>

// Local includes

#include "setup.h"
#include "facescansettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class MaintenanceDlg::MaintenanceDlgPriv
{
public:

    enum Operation
    {
        NewItems = 0,
        Thumbnails,
        FingerPrints,
        Duplicates,
        Metadata,
        FaceDetection,
        Stretch
    };

public:

    MaintenanceDlgPriv() :
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerPrints(0),
        metadataSetup(0),
        faceScannedHandling(0),
        hbox(0),
        hbox2(0),
        hbox3(0),
        similarity(0),
        expanderBox(0)
    {
    }

    static const QString configGroupName;

    QLabel*              logo;
    QLabel*              title;
    QCheckBox*           scanThumbs;
    QCheckBox*           scanFingerPrints;
    QPushButton*         metadataSetup;
    QComboBox*           faceScannedHandling;
    KHBox*               hbox;
    KHBox*               hbox2;
    KHBox*               hbox3;
    KIntNumInput*        similarity;
    RExpanderBox*        expanderBox;
};

const QString MaintenanceDlg::MaintenanceDlgPriv::configGroupName("MaintenanceDlg Settings");

MaintenanceDlg::MaintenanceDlg(QWidget* parent)
    : KDialog(parent), d(new MaintenanceDlgPriv)
{
    setHelp("digikam");
    setCaption(i18n("Maintenance"));
    setButtons(Ok|Help|Cancel);
    setDefaultButton(Cancel);

    QWidget* page      = new QWidget(this);
    setMainWidget(page);

    QGridLayout* grid  = new QGridLayout(page);

    d->logo            = new QLabel(page);
    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->title           = new QLabel(i18n("<qt><b>Select Maintenance Operations to Process</b></qt>"), page);
    d->expanderBox     = new RExpanderBox(page);
    KSeparator* line   = new KSeparator(Qt::Horizontal);

    d->expanderBox->insertItem(MaintenanceDlgPriv::NewItems, new QLabel(i18n("<qt><i>no option</i></qt>")), SmallIcon("view-refresh"),
                               i18n("Scan for new items"), "NewItems", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::NewItems, true);

    d->scanThumbs       = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(MaintenanceDlgPriv::Thumbnails, d->scanThumbs, SmallIcon("view-process-all"), i18n("Rebuild Thumbnails"), "Thumbnails", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::Thumbnails, true);

    d->scanFingerPrints = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(MaintenanceDlgPriv::FingerPrints, d->scanFingerPrints, SmallIcon("run-build"), i18n("Rebuild Finger-prints"), "Fingerprints", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::FingerPrints, true);

    d->hbox        = new KHBox;
    new QLabel(i18n("Similarity (in percents): "), d->hbox);
    QWidget* space = new QWidget(d->hbox);
    d->hbox->setStretchFactor(space, 10);
    d->similarity  = new KIntNumInput(d->hbox);
    d->similarity->setValue(90);
    d->similarity->setRange(0, 100, 1);
    d->similarity->setSliderEnabled(false);
    d->expanderBox->insertItem(MaintenanceDlgPriv::Duplicates, d->hbox, SmallIcon("tools-wizard"), i18n("Find Duplicates Items"), "Duplicates", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::Duplicates, true);

    d->hbox2         = new KHBox;
    new QLabel(i18n("Check metadata setup panel for details: "), d->hbox2);
    QWidget* space2  = new QWidget(d->hbox2);
    d->hbox2->setStretchFactor(space2, 10);
    d->metadataSetup = new QPushButton(i18n("Settings..."), d->hbox2);
    d->expanderBox->insertItem(MaintenanceDlgPriv::Metadata, d->hbox2, SmallIcon("run-build-file"),
                               i18n("Sync image metadata with Database"), "Metadata", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::Metadata, true);

    d->hbox3               = new KHBox;
    new QLabel(i18n("Faces data management: "), d->hbox3);
    QWidget* space3        = new QWidget(d->hbox3);
    d->hbox2->setStretchFactor(space3, 10);
    d->faceScannedHandling = new QComboBox(d->hbox3);
    d->faceScannedHandling->addItem(i18n("Skip images already scanned"),          FaceScanSettings::Skip);
    d->faceScannedHandling->addItem(i18n("Scan again and merge results"),         FaceScanSettings::Merge);
    d->faceScannedHandling->addItem(i18n("Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->expanderBox->insertItem(MaintenanceDlgPriv::FaceDetection, d->hbox3, SmallIcon("edit-image-face-detect"),
                               i18n("Face Detection"), "FaceDetection", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::FaceDetection, true);

    d->expanderBox->insertStretch(MaintenanceDlgPriv::Stretch);

    grid->addWidget(d->logo,        0, 0, 1, 1);
    grid->addWidget(d->title,       0, 1, 1, 1);
    grid->addWidget(line,           1, 1, 1, 1);
    grid->addWidget(d->expanderBox, 2, 0, 3, 2);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));

    connect(d->expanderBox, SIGNAL(signalItemToggled(int, bool)),
            this, SLOT(slotItemToggled(int, bool)));

    connect(d->metadataSetup, SIGNAL(clicked()),
            this, SLOT(slotMetadataSetup()));

    setMinimumSize(500, 300);
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
    prm.newItems                            = d->expanderBox->isChecked(MaintenanceDlgPriv::NewItems);
    prm.thumbnails                          = d->expanderBox->isChecked(MaintenanceDlgPriv::Thumbnails);
    prm.scanThumbs                          = d->scanThumbs->isChecked();
    prm.fingerPrints                        = d->expanderBox->isChecked(MaintenanceDlgPriv::FingerPrints);
    prm.scanFingerPrints                    = d->scanFingerPrints->isChecked();
    prm.duplicates                          = d->expanderBox->isChecked(MaintenanceDlgPriv::Duplicates);
    prm.similarity                          = d->similarity->value();
    prm.metadata                            = d->expanderBox->isChecked(MaintenanceDlgPriv::Metadata);
    prm.faceDetection                       = d->expanderBox->isChecked(MaintenanceDlgPriv::FaceDetection);
    prm.faceSettings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)d->faceScannedHandling->currentIndex();
    return prm;
}

void MaintenanceDlg::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->readSettings(group);

    MaintenanceSettings prm;

    d->expanderBox->setChecked(MaintenanceDlgPriv::NewItems,      group.readEntry("NewItems",      prm.newItems));
    d->expanderBox->setChecked(MaintenanceDlgPriv::Thumbnails,    group.readEntry("Thumbnails",    prm.thumbnails));
    d->scanThumbs->setChecked(group.readEntry("ScanThumbs",                        prm.scanThumbs));
    d->expanderBox->setChecked(MaintenanceDlgPriv::FingerPrints,  group.readEntry("FingerPrints",  prm.fingerPrints));
    d->scanFingerPrints->setChecked(group.readEntry("ScanFingerPrints",            prm.scanFingerPrints));
    d->expanderBox->setChecked(MaintenanceDlgPriv::Duplicates,    group.readEntry("Duplicates",    prm.duplicates));
    d->similarity->setValue(group.readEntry("Similarity",                          prm.similarity));
    d->expanderBox->setChecked(MaintenanceDlgPriv::Metadata,      group.readEntry("Metadata",      prm.metadata));
    d->expanderBox->setChecked(MaintenanceDlgPriv::FaceDetection, group.readEntry("FaceDetection", prm.faceDetection));
    d->faceScannedHandling->setCurrentIndex(group.readEntry("FaceScannedHandling", (int)prm.faceSettings.alreadyScannedHandling));

    for (int i = MaintenanceDlgPriv::NewItems ; i < MaintenanceDlgPriv::Stretch ; ++i)
        slotItemToggled(i, d->expanderBox->isChecked(i));
}

void MaintenanceDlg::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->expanderBox->writeSettings(group);

    MaintenanceSettings prm   = settings();

    group.writeEntry("NewItems",            prm.newItems);
    group.writeEntry("Thumbnails",          prm.thumbnails);
    group.writeEntry("ScanThumbs",          prm.scanThumbs);
    group.writeEntry("FingerPrints",        prm.fingerPrints);
    group.writeEntry("ScanFingerPrints",    prm.scanFingerPrints);
    group.writeEntry("Duplicates",          prm.duplicates);
    group.writeEntry("Similarity",          prm.similarity);
    group.writeEntry("Metadata",            prm.metadata);
    group.writeEntry("FaceDetection",       prm.faceDetection);
    group.writeEntry("FaceScannedHandling", (int)prm.faceSettings.alreadyScannedHandling);
}

void MaintenanceDlg::slotItemToggled(int index, bool b)
{
    switch(index)
    {
        case MaintenanceDlgPriv::Thumbnails:
            d->scanThumbs->setEnabled(b);
            break;
        case MaintenanceDlgPriv::FingerPrints:
            d->scanFingerPrints->setEnabled(b);
            break;
        case MaintenanceDlgPriv::Duplicates:
            d->hbox->setEnabled(b);
            break;
        case MaintenanceDlgPriv::Metadata:
            d->hbox2->setEnabled(b);
            break;
        case MaintenanceDlgPriv::FaceDetection:
            d->hbox3->setEnabled(b);
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

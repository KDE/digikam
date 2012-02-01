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
#include <QCheckBox>
#include <QGridLayout>

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
        Stretch
    };

public:

    MaintenanceDlgPriv() :
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerprints(0),
        hbox(0),
        similarity(0),
        expanderBox(0)
    {
    }

    QLabel*       logo;
    QLabel*       title;
    QCheckBox*    scanThumbs;
    QCheckBox*    scanFingerprints;
    KHBox*        hbox;
    KIntNumInput* similarity;
    RExpanderBox* expanderBox;
};

MaintenanceDlg::MaintenanceDlg(QWidget* parent)
    : KDialog(parent), d(new MaintenanceDlgPriv)
{
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

    d->scanFingerprints = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(MaintenanceDlgPriv::FingerPrints, d->scanFingerprints, SmallIcon("run-build"), i18n("Rebuild Finger-prints"), "Fingerprints", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::FingerPrints, true);

    d->hbox    = new KHBox;
    new QLabel(i18n("Similarity (in percents): "), d->hbox);
    QWidget* space = new QWidget(d->hbox);
    d->hbox->setStretchFactor(space, 10);
    d->similarity  = new KIntNumInput(d->hbox);
    d->similarity->setValue(90);
    d->similarity->setRange(0, 100, 1);
    d->similarity->setSliderEnabled(false);
    d->expanderBox->insertItem(MaintenanceDlgPriv::Duplicates, d->hbox, SmallIcon("tools-wizard"), i18n("Find Duplicates Items"), "Duplicates", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::Duplicates, true);

    d->expanderBox->insertItem(MaintenanceDlgPriv::Metadata, new QLabel(i18n("<qt><i>no option</i></qt>")), SmallIcon("run-build-file"),
                               i18n("Sync image metadata with Database"), "Metadata", false);
    d->expanderBox->setCheckBoxVisible(MaintenanceDlgPriv::Metadata, true);

    d->expanderBox->insertStretch(MaintenanceDlgPriv::Stretch);

    grid->addWidget(d->logo,        0, 0, 1, 1);
    grid->addWidget(d->title,       0, 1, 1, 1);
    grid->addWidget(line,           1, 1, 1, 1);
    grid->addWidget(d->expanderBox, 2, 0, 3, 2);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(1, 10);

    connect(d->expanderBox, SIGNAL(signalItemToggled(int, bool)),
            this, SLOT(slotItemToggled(int, bool)));

    setMinimumSize(500, 300);
    adjustSize();
}

MaintenanceDlg::~MaintenanceDlg()
{
    delete d;
}

MaintenanceSettings MaintenanceDlg::settings() const
{
    MaintenanceSettings settings;
    settings.newItems         = d->expanderBox->isChecked(MaintenanceDlgPriv::NewItems);
    settings.thumbnails       = d->expanderBox->isChecked(MaintenanceDlgPriv::Thumbnails);
    settings.scanThumbs       = d->scanThumbs->isChecked();
    settings.fingerPrints     = d->expanderBox->isChecked(MaintenanceDlgPriv::FingerPrints);
    settings.scanFingerPrints = d->scanFingerprints->isChecked();
    settings.duplicates       = d->expanderBox->isChecked(MaintenanceDlgPriv::Duplicates);
    settings.similarity       = d->similarity->value();
    settings.metadata         = d->expanderBox->isChecked(MaintenanceDlgPriv::Metadata);
    return settings;
}

void MaintenanceDlg::slotItemToggled(int index, bool b)
{
    switch(index)
    {
        case MaintenanceDlgPriv::Thumbnails:
            d->scanThumbs->setEnabled(b);
            break;
        case MaintenanceDlgPriv::FingerPrints:
            d->scanFingerprints->setEnabled(b);
            break;
        case MaintenanceDlgPriv::Duplicates:
            d->hbox->setEnabled(b);
            break;
        default :  // NewItems, Metadata
            break;
    }
}

}  // namespace Digikam

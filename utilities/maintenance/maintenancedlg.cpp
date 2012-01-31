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

#include <QHeaderView>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTreeWidget>

// Libkdcraw includes

#include <libkdcraw/rexpanderbox.h>

// KDE includes

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksqueezedtextlabel.h>

using namespace KDcrawIface;

namespace Digikam
{

class MaintenanceDlg::MaintenanceDlgPriv
{
public:

    MaintenanceDlgPriv() :
        logo(0),
        title(0),
        scanThumbs(0),
        scanFingerprints(0),
        expanderBox(0)
    {
    }

    QLabel*                logo;
    QLabel*                title;
    QCheckBox*             scanThumbs;
    QCheckBox*             scanFingerprints;
    RExpanderBoxExclusive* expanderBox;
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
                       .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->title           = new QLabel(i18n("Select Maintenance Operations to Process"), page);
    
    d->expanderBox     = new RExpanderBoxExclusive(page);
    d->expanderBox->setIsToolBox(true);

    d->scanThumbs       = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(0, d->scanThumbs, i18n("Rebuild Thumbnails"), "Thumbnails", false);
    d->expanderBox->setCheckBoxVisible(0, true);

    d->scanFingerprints = new QCheckBox(i18n("Scan for changed or non-cataloged items (faster)"));
    d->expanderBox->insertItem(1, d->scanFingerprints, i18n("Rebuild Finger-prints"), "Fingerprints", false);
    d->expanderBox->setCheckBoxVisible(1, true);

    d->expanderBox->insertStretch(2);

    grid->addWidget(d->logo,        0, 0, 3, 1);
    grid->addWidget(d->title,          0, 1, 1, 1);
    grid->addWidget(d->expanderBox, 1, 1, 3, 1);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(1, 10);

    adjustSize();
}

MaintenanceDlg::~MaintenanceDlg()
{
    delete d;
}

MaintenanceSettings MaintenanceDlg::settings() const
{
    MaintenanceSettings settings;
    settings.thumbnails       = d->expanderBox->isChecked(0);
    settings.scanThumbs       = d->scanThumbs->isChecked();
    settings.fingerPrints     = d->expanderBox->isChecked(1);
    settings.scanFingerPrints = d->scanFingerprints->isChecked();
    return settings;
}

}  // namespace Digikam

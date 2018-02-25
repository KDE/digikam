/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatapage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "metadatasettings.h"

namespace Digikam
{

class MetadataPage::Private
{
public:

    explicit Private()
      : doNothing(0),
        storeInFiles(0),
        metadataStorage(0)
    {
    }

    QRadioButton* doNothing;
    QRadioButton* storeInFiles;
    QButtonGroup* metadataStorage;
};

MetadataPage::MetadataPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Metadata Storage to Files</b>")),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DVBox* const vbox    = new DVBox(this);
    QLabel* const label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here if you want to store the information assigned to items in digiKam in the files' "
                         "metadata, to improve interoperability with other photo management programs:</p>"
                         "</qt>"));

    QWidget* const btns      = new QWidget(vbox);
    QVBoxLayout* const vlay  = new QVBoxLayout(btns);

    d->metadataStorage = new QButtonGroup(btns);
    d->doNothing       = new QRadioButton(btns);
    d->doNothing->setText(i18n("Do nothing"));
    d->doNothing->setChecked(true);
    d->metadataStorage->addButton(d->doNothing);

    d->storeInFiles    = new QRadioButton(btns);
    d->storeInFiles->setText(i18n("Add information to files"));
    d->metadataStorage->addButton(d->storeInFiles);

    vlay->addWidget(d->doNothing);
    vlay->addWidget(d->storeInFiles);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> recording information to the files' metadata can slow down photo "
                         "management operations.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("format-text-code")));
}

MetadataPage::~MetadataPage()
{
    delete d;
}

void MetadataPage::saveSettings()
{
    MetadataSettingsContainer settings;
    settings.saveTags       = d->storeInFiles->isChecked();
    settings.saveComments   = d->storeInFiles->isChecked();
    settings.saveDateTime   = d->storeInFiles->isChecked();
    settings.saveRating     = d->storeInFiles->isChecked();
    settings.savePickLabel  = d->storeInFiles->isChecked();
    settings.saveColorLabel = d->storeInFiles->isChecked();
    settings.saveTemplate   = d->storeInFiles->isChecked();
    MetadataSettings::instance()->setSettings(settings);
}

} // namespace Digikam

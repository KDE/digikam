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

#include "previewpage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"

namespace Digikam
{

class PreviewPage::Private
{
public:

    explicit Private()
      : loadReduced(0),
        loadimage(0),
        previewBehavior(0)
    {
    }

    QRadioButton* loadReduced;
    QRadioButton* loadimage;
    QButtonGroup* previewBehavior;
};

PreviewPage::PreviewPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Preview Behavior</b>")),
      d(new Private)
{
    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DVBox* const vbox       = new DVBox(this);
    QLabel* const label1    = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here how images are displayed in preview mode and on the light table:</p>"
                         "</qt>"));

    QWidget* const btns     = new QWidget(vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(btns);

    d->previewBehavior      = new QButtonGroup(btns);
    d->loadReduced          = new QRadioButton(btns);
    d->loadReduced->setText(i18n("Load reduced version of image"));
    d->loadReduced->setChecked(true);
    d->previewBehavior->addButton(d->loadReduced);

    d->loadimage            = new QRadioButton(btns);
    d->loadimage->setText(i18n("Load image"));
    d->previewBehavior->addButton(d->loadimage);

    vlay->addWidget(d->loadReduced);
    vlay->addWidget(d->loadimage);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> loading a reduced version of an image is faster but can differ "
                         "from the original, especially with Raw. In this case, a JPEG version "
                         "of Raw pre-processed by camera is loaded, instead of the real image data. This JPEG "
                         "image is embedded in the file metadata and is used by the camera to display "
                         "a Raw image faster to a TV screen.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-preview")));
}

PreviewPage::~PreviewPage()
{
    delete d;
}

void PreviewPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group        = config->group(QLatin1String("Album Settings"));
    group.writeEntry(QLatin1String("Preview Load Full Image Size"), d->loadimage->isChecked());

    config->sync();
}

}   // namespace Digikam

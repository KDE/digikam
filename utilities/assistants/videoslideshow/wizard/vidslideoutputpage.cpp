/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "vidslideoutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "dfileselector.h"
#include "dlayoutbox.h"

namespace Digikam
{

class VidSlideOutputPage::Private
{
public:

    Private()
      : destUrl(0),
        openInPlayer(0)
    {
    }

    DFileSelector* destUrl;
    QCheckBox*     openInPlayer;
};

VidSlideOutputPage::VidSlideOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    DVBox* const vbox        = new DVBox(this);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------

    DVBox* const hbox1       = new DVBox(vbox);
    hbox1->setContentsMargins(QMargins());
    hbox1->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const textLabel1 = new QLabel(hbox1);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Output video file:"));

    d->destUrl = new DFileSelector(hbox1);
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    d->openInPlayer          = new QCheckBox(vbox);
    d->openInPlayer->setText(i18n("Open in video player"));

    // --------------------

    QWidget* const spacer    = new QWidget(vbox);
    vbox->setStretchFactor(spacer, 10);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-video")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));
}

VidSlideOutputPage::~VidSlideOutputPage()
{
    delete d;
}

void VidSlideOutputPage::initializePage()
{
    VidSlideWizard* const wizard = dynamic_cast<VidSlideWizard*>(assistant());

    if (!wizard)
        return;

    VidSlideSettings* const settings  = wizard->settings();

    d->destUrl->setFileDlgPath(settings->outputVideo.toLocalFile());
    d->openInPlayer->setChecked(settings->openInPlayer);
}

bool VidSlideOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    VidSlideWizard* const wizard = dynamic_cast<VidSlideWizard*>(assistant());

    if (!wizard)
        return false;

    VidSlideSettings* const settings  = wizard->settings();

    settings->outputVideo  = QUrl::fromLocalFile(d->destUrl->fileDlgPath());
    settings->openInPlayer = d->openInPlayer->isChecked();

    return true;
}

bool VidSlideOutputPage::isComplete() const
{
    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace Digikam

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
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "dfileselector.h"

namespace Digikam
{

class VidSlideOutputPage::Private
{
public:

    Private(QWizard* const dialog)
      : destUrl(0),
        openInPlayer(0),
        wizard(0),
        settings(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    DFileSelector*    destUrl;
    QCheckBox*        openInPlayer;
    VidSlideWizard*   wizard;
    VidSlideSettings* settings;
};

VidSlideOutputPage::VidSlideOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main       = new QWidget(this);

    // --------------------

    QLabel* const fileLabel  = new QLabel(main);
    fileLabel->setWordWrap(false);
    fileLabel->setText(i18n("Output video file:"));

    d->destUrl = new DFileSelector(main);
    d->destUrl->setFileDlgMode(QFileDialog::AnyFile);
    d->destUrl->setFileDlgTitle(i18n("Output Video File"));
    fileLabel->setBuddy(d->destUrl);

    // --------------------

    d->openInPlayer           = new QCheckBox(main);
    d->openInPlayer->setText(i18n("Open in video player"));

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(fileLabel,       0, 0, 1, 1);
    grid->addWidget(d->destUrl,      0, 1, 1, 1);
    grid->addWidget(d->openInPlayer, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);

    setPageWidget(main);
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
    d->destUrl->setFileDlgPath(d->settings->outputVideo.toLocalFile());
    d->openInPlayer->setChecked(d->settings->openInPlayer);
}

bool VidSlideOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    d->settings->outputVideo  = QUrl::fromLocalFile(d->destUrl->fileDlgPath());
    d->settings->openInPlayer = d->openInPlayer->isChecked();

    return true;
}

bool VidSlideOutputPage::isComplete() const
{
    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace Digikam

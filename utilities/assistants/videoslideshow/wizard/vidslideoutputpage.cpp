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
#include <QComboBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// QtAv includes

#include <QtAV/AVMuxer.h>

// Local includes

#include "vidslidewizard.h"
#include "dfileselector.h"
#include "filesaveconflictbox.h"

using namespace QtAV;

namespace Digikam
{

class VidSlideOutputPage::Private
{
public:

    Private(QWizard* const dialog)
      : destUrl(0),
        conflictBox(0),
        playerVal(0),
        formatVal(0),
        wizard(0),
        settings(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    DFileSelector*       destUrl;
    FileSaveConflictBox* conflictBox;
    QComboBox*           playerVal;
    QComboBox*           formatVal;
    VidSlideWizard*      wizard;
    VidSlideSettings*    settings;
};

VidSlideOutputPage::VidSlideOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main       = new QWidget(this);

    // --------------------

    QLabel* const formatLabel = new QLabel(main);
    formatLabel->setWordWrap(false);
    formatLabel->setText(i18n("Media Container Format:"));
    d->formatVal              = new QComboBox(main);
    d->formatVal->setEditable(false);

    QMap<VidSlideSettings::VidFormat, QString> map                = VidSlideSettings::videoFormatNames();
    QMap<VidSlideSettings::VidFormat, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->formatVal->addItem(it.value(), (int)it.key());

        // Disable format entry if QtAV/ffmpeg format is not available.

        VidSlideSettings tmp;
        tmp.vFormat = (VidSlideSettings::VidFormat)it.key();

        if (!AVMuxer::supportedExtensions().contains(tmp.videoFormat()))
            d->formatVal->setItemData((int)it.key(), false, Qt::UserRole-1);

        ++it;
    }

    formatLabel->setBuddy(d->formatVal);

    // --------------------

    QLabel* const fileLabel = new QLabel(main);
    fileLabel->setWordWrap(false);
    fileLabel->setText(i18n("Destination Folder:"));

    d->destUrl              = new DFileSelector(main);
    d->destUrl->setFileDlgMode(DFileDialog::DirectoryOnly);
    d->destUrl->setFileDlgTitle(i18n("Destination Folder"));
    d->destUrl->lineEdit()->setPlaceholderText(i18n("Output Destination Path"));
    fileLabel->setBuddy(d->destUrl);

    // --------------------

    QLabel* const outputLbl = new QLabel(main);
    outputLbl->setText(i18n("The video output file name will be generated automatically."));
    d->conflictBox          = new FileSaveConflictBox(main);

    // --------------------

    QLabel* const playerLabel = new QLabel(main);
    playerLabel->setWordWrap(false);
    playerLabel->setText(i18n("Open in Player:"));
    d->playerVal              = new QComboBox(main);
    d->playerVal->setEditable(false);

    QMap<VidSlideSettings::VidPlayer, QString> map2                = VidSlideSettings::videoPlayerNames();
    QMap<VidSlideSettings::VidPlayer, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->playerVal->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    playerLabel->setBuddy(d->playerVal);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(formatLabel,     0, 0, 1, 1);
    grid->addWidget(d->formatVal,    0, 1, 1, 1);
    grid->addWidget(fileLabel,       1, 0, 1, 1);
    grid->addWidget(d->destUrl,      1, 1, 1, 1);
    grid->addWidget(outputLbl,       2, 0, 1, 2);
    grid->addWidget(d->conflictBox,  3, 0, 1, 2);
    grid->addWidget(playerLabel,     4, 0, 1, 1);
    grid->addWidget(d->playerVal,    4, 1, 1, 1);
    grid->setRowStretch(5, 10);

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
    d->formatVal->setCurrentIndex(d->settings->vFormat);
    d->destUrl->setFileDlgPath(d->settings->outputDir.toLocalFile());
    d->conflictBox->setConflictRule(d->settings->conflictRule);
    d->playerVal->setCurrentIndex(d->settings->outputPlayer);
}

bool VidSlideOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    d->settings->vFormat      = (VidSlideSettings::VidFormat)d->formatVal->currentIndex();
    d->settings->outputDir    = QUrl::fromLocalFile(d->destUrl->fileDlgPath());
    d->settings->conflictRule = d->conflictBox->conflictRule();
    d->settings->outputPlayer = (VidSlideSettings::VidPlayer)d->playerVal->currentIndex();

    return true;
}

bool VidSlideOutputPage::isComplete() const
{
    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace Digikam

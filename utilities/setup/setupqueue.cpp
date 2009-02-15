/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-12
 * Description : setup Queue Manager tab.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupqueue.h"
#include "setupqueue.moc"

// Qt includes.

#include <QLabel>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QGroupBox>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes.

#include "album.h"
#include "kipiuploadwidget.h"

namespace Digikam
{

class SetupQueuePriv
{
public:

    SetupQueuePriv()
    {
        conflictLabel       = 0;
        conflictButtonGroup = 0;
        overwriteButton     = 0;
        promptButton        = 0;
        uploadWidget        = 0;
    }

    QLabel           *conflictLabel;
    QLabel           *uploadLabel;

    QButtonGroup     *conflictButtonGroup;

    QRadioButton     *overwriteButton;
    QRadioButton     *promptButton;

    KipiUploadWidget *uploadWidget;
};

SetupQueue::SetupQueue(QWidget* parent)
          : QScrollArea(parent), d(new SetupQueuePriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *saveSettingsGroup = new QGroupBox(i18n("Save Settings"), panel);
    QVBoxLayout *gLayout         = new QVBoxLayout(saveSettingsGroup);

    d->uploadLabel         = new QLabel(i18n("Album to Host Processed Items:"), saveSettingsGroup);
    d->uploadWidget        = new KipiUploadWidget(0, saveSettingsGroup);

    d->conflictLabel       = new QLabel(i18n("If Target File Exists:"), saveSettingsGroup);
    QWidget *conflictBox   = new QWidget(saveSettingsGroup);
    QVBoxLayout *vlay      = new QVBoxLayout(conflictBox);
    d->conflictButtonGroup = new QButtonGroup(conflictBox);
    d->overwriteButton     = new QRadioButton(i18n("Overwrite automatically"), conflictBox);
    d->promptButton        = new QRadioButton(i18n("Open rename-file dialog"), conflictBox);
    d->conflictButtonGroup->addButton(d->overwriteButton, OVERWRITE);
    d->conflictButtonGroup->addButton(d->promptButton,    ASKTOUSER);
    d->conflictButtonGroup->setExclusive(true);
    d->overwriteButton->setChecked(true);

    vlay->addWidget(d->overwriteButton);
    vlay->addWidget(d->promptButton);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    gLayout->addWidget(d->uploadLabel);
    gLayout->addWidget(d->uploadWidget);
    gLayout->addWidget(d->conflictLabel);
    gLayout->addWidget(conflictBox);
    gLayout->setMargin(KDialog::spacingHint());
    gLayout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->addWidget(saveSettingsGroup);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
}

SetupQueue::~SetupQueue()
{
    delete d;
}

void SetupQueue::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Batch Queue Manager Settings"));

    int btn = group.readEntry("Conflict Rule", (int)(OVERWRITE));
    d->conflictButtonGroup->button(btn)->setChecked(true);

    PAlbum * palbum = dynamic_cast<PAlbum*>(AlbumManager::instance()->currentAlbum());
    d->uploadWidget->setCurrentAlbumUrl(group.readEntry("Processes Items Url", palbum ? palbum->fileUrl() : KUrl()));
}

void SetupQueue::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Batch Queue Manager Settings"));

    group.writeEntry("Conflict Rule",       (int)d->conflictButtonGroup->checkedId());
    group.writeEntry("Processes Items Url", d->uploadWidget->currentAlbumUrl());
    config->sync();
}

}  // namespace Digikam

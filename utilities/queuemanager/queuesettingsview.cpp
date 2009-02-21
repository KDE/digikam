/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuesettingsview.h"
#include "queuesettingsview.moc"

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

class QueueSettingsViewPriv
{

public:

    QueueSettingsViewPriv()
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

QueueSettingsView::QueueSettingsView(QWidget *parent)
                 : QScrollArea(parent), d(new QueueSettingsViewPriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    d->uploadLabel         = new QLabel(i18n("Album to Host Processed Items:"), panel);
    d->uploadWidget        = new KipiUploadWidget(0, panel);

    d->conflictLabel       = new QLabel(i18n("If Target File Exists:"), panel);
    QWidget *conflictBox   = new QWidget(panel);
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
    vlay->setMargin(0);
    vlay->setSpacing(0);

    layout->addWidget(d->uploadLabel);
    layout->addWidget(d->uploadWidget);
    layout->addWidget(d->conflictLabel);
    layout->addWidget(conflictBox);
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
}

QueueSettingsView::~QueueSettingsView()
{
    delete d;
}

}  // namespace Digikam

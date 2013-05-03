/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen settings widget
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes

#include <kglobalsettings.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "dxmlguiwindow.h"
#include "fullscreensettings.h"

namespace Digikam
{

class FullScreenSettings::Private
{
public:

    Private()
    {
        options      = FS_DEFAULT;
        hideToolBar  = 0;
        hideThumbBar = 0;
    }

    int        options;

    QCheckBox* hideToolBar;
    QCheckBox* hideThumbBar;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->options              = options;
    QVBoxLayout* const vlay = new QVBoxLayout(this);
    d->hideToolBar          = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),  this);
    d->hideThumbBar         = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), this);

    if (!(options & FS_TOOLBAR))  d->hideToolBar->hide();
    if (!(options & FS_THUMBBAR)) d->hideThumbBar->hide();

    vlay->addWidget(d->hideToolBar);
    vlay->addWidget(d->hideThumbBar);
    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());
}

FullScreenSettings::~FullScreenSettings()
{
    delete d;
}

void FullScreenSettings::readSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        d->hideToolBar->setChecked(group.readEntry(s_configFullScreenHideToolBarEntry,  false));

    if (d->options & FS_THUMBBAR)
        d->hideThumbBar->setChecked(group.readEntry(s_configFullScreenHideThumbBarEntry, true));
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        group.writeEntry(s_configFullScreenHideToolBarEntry,  d->hideToolBar->isChecked());

    if (d->options & FS_THUMBBAR)
        group.writeEntry(s_configFullScreenHideThumbBarEntry, d->hideThumbBar->isChecked());

    group.sync();
}

} // namespace Digikam

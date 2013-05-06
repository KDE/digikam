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
        hideToolBars = 0;
        hideThumbBar = 0;
    }

    int        options;

    QCheckBox* hideToolBars;
    QCheckBox* hideThumbBar;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QGroupBox(i18n("Full-screen Options"), parent), d(new Private)
{
    d->options               = options;
    QVBoxLayout* const vlay  = new QVBoxLayout(this);
    d->hideToolBars          = new QCheckBox(i18n("H&ide toolbars"),  this);
    d->hideToolBars->setWhatsThis(i18n("Hide all toolbars when window switch in full-screen mode."));

    d->hideThumbBar          = new QCheckBox(i18n("Hide &thumbbar"), this);
    d->hideThumbBar->setWhatsThis(i18n("Hide thumbbar view when window switch in full-screen mode."));

    if (!(options & FS_TOOLBAR))  d->hideToolBars->hide();
    if (!(options & FS_THUMBBAR)) d->hideThumbBar->hide();

    vlay->addWidget(d->hideToolBars);
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
        d->hideToolBars->setChecked(group.readEntry(s_configFullScreenHideToolBarEntry,  false));

    if (d->options & FS_THUMBBAR)
        d->hideThumbBar->setChecked(group.readEntry(s_configFullScreenHideThumbBarEntry, true));
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBAR)
        group.writeEntry(s_configFullScreenHideToolBarEntry,  d->hideToolBars->isChecked());

    if (d->options & FS_THUMBBAR)
        group.writeEntry(s_configFullScreenHideThumbBarEntry, d->hideThumbBar->isChecked());

    group.sync();
}

} // namespace Digikam

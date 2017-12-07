/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen settings widget
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

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
        options      = FS_NONE;
        hideToolBars = 0;
        hideThumbBar = 0;
        hideSideBars = 0;
    }

    int        options;

    QCheckBox* hideToolBars;
    QCheckBox* hideThumbBar;
    QCheckBox* hideSideBars;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QGroupBox(i18n("Full-screen Options"), parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->options               = options;
    QVBoxLayout* const vlay  = new QVBoxLayout(this);
    d->hideToolBars          = new QCheckBox(i18n("H&ide toolbars"),  this);
    d->hideToolBars->setWhatsThis(i18n("Hide all toolbars when window switch in full-screen mode."));

    d->hideThumbBar          = new QCheckBox(i18n("Hide &thumbbar"), this);
    d->hideThumbBar->setWhatsThis(i18n("Hide thumbbar view when window switch in full-screen mode."));

    d->hideSideBars          = new QCheckBox(i18n("Hide &sidebars"), this);
    d->hideSideBars->setWhatsThis(i18n("Hide all side-bars when window switch in full-screen mode."));

    vlay->addWidget(d->hideToolBars);
    vlay->addWidget(d->hideThumbBar);
    vlay->addWidget(d->hideSideBars);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(0);

    if (!(options & FS_TOOLBARS)) d->hideToolBars->hide();
    if (!(options & FS_THUMBBAR)) d->hideThumbBar->hide();
    if (!(options & FS_SIDEBARS)) d->hideSideBars->hide();
}

FullScreenSettings::~FullScreenSettings()
{
    delete d;
}

void FullScreenSettings::readSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
        d->hideToolBars->setChecked(group.readEntry(s_configFullScreenHideToolBarsEntry,  false));

    if (d->options & FS_THUMBBAR)
        d->hideThumbBar->setChecked(group.readEntry(s_configFullScreenHideThumbBarEntry, true));

    if (d->options & FS_SIDEBARS)
        d->hideSideBars->setChecked(group.readEntry(s_configFullScreenHideSideBarsEntry, false));
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
        group.writeEntry(s_configFullScreenHideToolBarsEntry,  d->hideToolBars->isChecked());

    if (d->options & FS_THUMBBAR)
        group.writeEntry(s_configFullScreenHideThumbBarEntry, d->hideThumbBar->isChecked());

    if (d->options & FS_SIDEBARS)
        group.writeEntry(s_configFullScreenHideSideBarsEntry,  d->hideSideBars->isChecked());

    group.sync();
}

} // namespace Digikam

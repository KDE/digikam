/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : a bar to indicate pending metadata
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "metadatastatusbar.h"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QDebug>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metadatasettings.h"
#include "dexpanderbox.h"

namespace Digikam
{

class MetadataStatusBar::Private
{
public:

    enum FilterStatus
    {
        None = 0,
        Match,
        NotMatch
    };

public:

    Private()
    {
        status      = None;
        info        = 0;
        applyBtn    = 0;
    }

    int               status;

    DAdjustableLabel* info;
    QToolButton*      applyBtn;
};

MetadataStatusBar::MetadataStatusBar(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QHBoxLayout* const vlay = new QHBoxLayout(this);

    d->applyBtn    = new QToolButton(this);
    d->applyBtn->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->applyBtn->setToolTip(i18n("Apply pending changes to metadata"));
    d->applyBtn->setFocusPolicy(Qt::NoFocus);
    d->applyBtn->setAutoRaise(true);
    d->applyBtn->setDisabled(true);

    d->info        = new DAdjustableLabel(this);
    d->info->setContextMenuPolicy(Qt::NoContextMenu);
    d->info->setAutoFillBackground(true);
    d->info->setFocusPolicy(Qt::NoFocus);
    d->info->setAdjustedText(i18n("No pending metadata synchronization"));

    d->info->setWhatsThis(i18n("If lazy synchronization is enabled in metadata settings, "
                               "the status bar will display the number of items waiting for synchronization"));

    vlay->addWidget(d->applyBtn);
    vlay->addWidget(d->info);
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->setContentsMargins(QMargins());
    vlay->setStretchFactor(d->info, 10);

    connect(MetadataSettings::instance(), SIGNAL(settingsChanged()), this,
            SLOT(slotSettingsChanged()));

    connect(d->applyBtn, SIGNAL(released()),
            MetadataHubMngr::instance(), SLOT(slotApplyPending()));

    connect(MetadataHubMngr::instance(), SIGNAL(signalPendingMetadata(int)),
            this, SLOT(slotSetPendingItems(int)));

    if (MetadataSettings::instance()->settings().useLazySync)
        this->show();
    else
        this->hide();
}

MetadataStatusBar::~MetadataStatusBar()
{
    delete d;
}

void MetadataStatusBar::slotSettingsChanged()
{
    if (MetadataSettings::instance()->settings().useLazySync)
        this->show();
    else
        this->hide();
}

void MetadataStatusBar::slotSetPendingItems(int number)
{
    if (number == 0)
    {
        d->info->setAdjustedText(i18n("No pending metadata synchronization"));
        d->applyBtn->setDisabled(true);
    }
    else
    {
        d->info->setAdjustedText(i18np("1 file awaits synchronization",
                                       "%1 files await synchronization",
                                       number));
        d->applyBtn->setDisabled(false);
    }
}

}  // namespace Digikam

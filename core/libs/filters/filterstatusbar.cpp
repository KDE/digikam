/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to indicate icon-view filters status
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Michael G. Hansen <mike at mghansen dot de>
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

#include "filterstatusbar.h"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"

namespace Digikam
{

class FilterStatusBar::Private
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
        resetBtn    = 0;
        settingsBtn = 0;
    }

    int               status;

    DAdjustableLabel* info;
    QToolButton*      resetBtn;
    QToolButton*      settingsBtn;

    ImageFilterSettings settings;
};

FilterStatusBar::FilterStatusBar(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QHBoxLayout* const vlay = new QHBoxLayout(this);

    d->info        = new DAdjustableLabel(this);
    d->info->setContextMenuPolicy(Qt::NoContextMenu);
    d->info->setAutoFillBackground(true);
    d->info->setFocusPolicy(Qt::NoFocus);
    d->info->setWhatsThis(i18n("Background color indicates the global image filter status, "
                               "encompassing all filter settings from the right sidebar.\n\n"
                               "NO COLOR: no filter is active, all items are visible.\n"
                               "RED: filtering is on, but no items match.\n"
                               "GREEN: filter(s) match(es) at least one item.\n\n"
                               "Move mouse cursor over this text to see more details about active filters.\n"
                               "Press the Reset button from the right side to clear all filter settings.\n"
                               "Press the Settings button from the right side to open the filters panel."));

    d->resetBtn    = new QToolButton(this);
    d->resetBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->resetBtn->setToolTip(i18n("Reset all active filters"));
    d->resetBtn->setFocusPolicy(Qt::NoFocus);
    d->resetBtn->setAutoRaise(true);

    d->settingsBtn = new QToolButton(this);
    d->settingsBtn->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    d->settingsBtn->setToolTip(i18n("Open filter settings panel"));
    d->settingsBtn->setFocusPolicy(Qt::NoFocus);
    d->settingsBtn->setAutoRaise(true);

    vlay->addWidget(d->info);
    vlay->addWidget(d->resetBtn);
    vlay->addWidget(d->settingsBtn);
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->setContentsMargins(QMargins());
    vlay->setStretchFactor(d->info, 10);

    connect(d->resetBtn, SIGNAL(released()),
            this, SIGNAL(signalResetFilters()));

    connect(d->settingsBtn, SIGNAL(released()),
            this, SIGNAL(signalPopupFiltersView()));
}

FilterStatusBar::~FilterStatusBar()
{
    delete d;
}

void FilterStatusBar::slotFilterMatches(bool match)
{
    QStringList filtersList;
    QString     message;

    if (d->settings.isFilteringByText())
    {
        filtersList.append(i18n("<br/><nobr><i>Text</i></nobr>"));
    }

    if (d->settings.isFilteringByTypeMime())
    {
        filtersList.append(i18n("<br/><nobr><i>Mime Type</i></nobr>"));
    }

    if (d->settings.isFilteringByGeolocation())
    {
        filtersList.append(i18n("<br/><nobr><i>Geolocation</i></nobr>"));
    }

    if (d->settings.isFilteringByRating())
    {
        filtersList.append(i18n("<br/><nobr><i>Rating</i></nobr>"));
    }

    if (d->settings.isFilteringByPickLabels())
    {
        filtersList.append(i18n("<br/><nobr><i>Pick Labels</i></nobr>"));
    }

    if (d->settings.isFilteringByColorLabels())
    {
        filtersList.append(i18n("<br/><nobr><i>Color Labels</i></nobr>"));
    }

    if (d->settings.isFilteringByTags())
    {
        filtersList.append(i18n("<br/><nobr><i>Tags</i></nobr>"));
    }

    if (filtersList.count() > 1)
    {
        message = i18n("<nobr><b>Active filters:</b></nobr>");
    }
    else
    {
        message = i18n("<nobr><b>Active filter:</b></nobr>");
    }

    message.append(filtersList.join(QString()));

    if (filtersList.isEmpty())
    {
        d->info->setAdjustedText(i18n("No active filter"));
        d->info->setToolTip(QString());
        d->resetBtn->setEnabled(false);
        d->status = Private::None;
    }
    else
    {
        if (filtersList.count() == 1)
        {
            d->info->setAdjustedText(i18n("One active filter"));
        }
        else
        {
            d->info->setAdjustedText(i18np("1 active filter", "%1 active filters", filtersList.count()));
        }

        d->info->setToolTip(message);
        d->resetBtn->setEnabled(true);
        d->status = match ? Private::Match : Private::NotMatch;
    }

    QPalette pal = palette();

    switch(d->status)
    {
        case Private::NotMatch:
            pal.setColor(backgroundRole(), QColor(255, 200, 200));
            pal.setColor(foregroundRole(), Qt::black);
            break;
        case Private::Match:
            pal.setColor(backgroundRole(), QColor(200, 255, 200));
            pal.setColor(foregroundRole(), Qt::black);
            break;
        default: // Private::None
            break;
    }

    d->info->setPalette(pal);

    update();
}

void FilterStatusBar::slotFilterSettingsChanged(const ImageFilterSettings& settings)
{
    d->settings = settings;
}

}  // namespace Digikam

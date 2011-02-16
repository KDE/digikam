/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumiconviewfilter.moc"

// Qt includes

#include <QMouseEvent>
#include <QLabel>

// KDE includes

#include <klocale.h>
#include <kdialog.h>

// Local includes

#include "statusled.h"

namespace Digikam
{

class AlbumIconViewFilter::AlbumIconViewFilterPriv
{
public:

    AlbumIconViewFilterPriv()
    {
        led = 0;
    }

    StatusLed*          led;
    ImageFilterSettings settings;
};

AlbumIconViewFilter::AlbumIconViewFilter(QWidget* parent)
    : KHBox(parent), d(new AlbumIconViewFilterPriv)
{
    d->led = new StatusLed(this);
    d->led->installEventFilter(this);
    d->led->setLedColor(StatusLed::Gray);
    d->led->setWhatsThis(i18n("This LED indicates the global image filter status, "
                              "encompassing all status-bar filters and all tag filters "
                              "from the right sidebar.\n\n"
                              "GRAY: no filter is active, all items are visible.\n"
                              "RED: filtering is on, but no items match.\n"
                              "GREEN: filter(s) match(es) at least one item.\n\n"
                              "Any mouse button click will reset all filters."));

    QLabel* space = new QLabel(this);

    setSpacing(KDialog::spacingHint());
    setMargin(0);
    setStretchFactor(space, 10);
}

AlbumIconViewFilter::~AlbumIconViewFilter()
{
    delete d;
}

void AlbumIconViewFilter::slotFilterMatches(bool match)
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
        d->led->setToolTip(i18n("No active filter"));
        d->led->setLedColor(StatusLed::Gray);
    }
    else
    {
        d->led->setToolTip(message);
        d->led->setLedColor(match ? StatusLed::Green : StatusLed::Red);
    }
}

void AlbumIconViewFilter::slotFilterSettingsChanged(const ImageFilterSettings& settings)
{
    d->settings = settings;
}

bool AlbumIconViewFilter::eventFilter(QObject* object, QEvent* e)
{
    QWidget* widget = static_cast<QWidget*>(object);

    if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* event = static_cast<QMouseEvent*>(e);

        if ( widget->rect().contains(event->pos()) && d->led->ledColor() != StatusLed::Gray)
        {
            // Reset all filters settings into Filters sidebar.
            emit signalResetFilters();
        }
    }

    return false;
}

}  // namespace Digikam

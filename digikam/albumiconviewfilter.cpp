/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "ddebug.h"
#include "statusled.h"
#include "albumsettings.h"
#include "searchtextbar.h"
#include "ratingfilter.h"
#include "mimefilter.h"
#include "albumiconviewfilter.h"
#include "albumiconviewfilter.moc"

namespace Digikam
{

class AlbumIconViewFilterPriv
{
public:

    AlbumIconViewFilterPriv()
    {
        textFilter       = 0;
        mimeFilter       = 0;
        ratingFilter     = 0;
        led              = 0;
    }

    StatusLed     *led;

    SearchTextBar *textFilter;

    MimeFilter    *mimeFilter;

    RatingFilter  *ratingFilter;
};

AlbumIconViewFilter::AlbumIconViewFilter(QWidget* parent)
                   : QHBox(parent)
{
    d = new AlbumIconViewFilterPriv;

    d->led = new StatusLed(this);
    d->led->installEventFilter(this);
    d->led->setLedColor(StatusLed::Gray);
    QWhatsThis::add(d->led, i18n("This LED indicates the global image filter status, "
                                 "encompassing all status-bar filters and all tag filters from the right sidebar.\n\n"
                                 "GRAY: no filter is active, all items are visible.\n"
                                 "RED: filtering is on, but no items match.\n"
                                 "GREEN: filter(s) matches at least one item.\n\n"
                                 "Any mouse button click will reset all filters."));

    d->textFilter = new SearchTextBar(this, "AlbumIconViewFilterTextFilter");
    d->textFilter->setEnableTextQueryCompletion(true);
    QToolTip::add(d->textFilter, i18n("Text quick filter (search)"));
    QWhatsThis::add(d->textFilter, i18n("Enter search patterns to quickly filter this view on file names, "
                                        "captions (comments), and tags"));

    d->mimeFilter   = new MimeFilter(this);
    d->ratingFilter = new RatingFilter(this);

    setSpacing(KDialog::spacingHint());
    setMargin(0);

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int, AlbumLister::RatingCondition)),
            this, SLOT(slotRatingFilterChanged(int, AlbumLister::RatingCondition)));

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SLOT(slotMimeTypeFilterChanged(int)));

    connect(d->textFilter, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotTextFilterChanged(const QString&)));

    connect(AlbumLister::instance(), SIGNAL(signalItemsTextFilterMatch(bool)),
            d->textFilter, SLOT(slotSearchResult(bool)));

    connect(AlbumLister::instance(), SIGNAL(signalItemsFilterMatch(bool)),
            this, SLOT(slotItemsFilterMatch(bool)));
}

AlbumIconViewFilter::~AlbumIconViewFilter()
{
    delete d;
}

void AlbumIconViewFilter::readSettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    d->ratingFilter->setRatingFilterCondition((Digikam::AlbumLister::RatingCondition)
                                              (settings->getRatingFilterCond()));
    /*
    Bug 181705: always enable filters
    d->ratingFilter->setEnabled(settings->getIconShowRating());
    d->textFilter->setEnabled(settings->getIconShowName()     ||
                              settings->getIconShowComments() ||
                              settings->getIconShowTags());
    */
}

void AlbumIconViewFilter::saveSettings()
{
    AlbumSettings::instance()->setRatingFilterCond(d->ratingFilter->ratingFilterCondition());
}

void AlbumIconViewFilter::slotRatingFilterChanged(int rating, AlbumLister::RatingCondition cond)
{
    AlbumLister::instance()->setRatingFilter(rating, cond);
}

void AlbumIconViewFilter::slotMimeTypeFilterChanged(int mimeTypeFilter)
{
    AlbumLister::instance()->setMimeTypeFilter(mimeTypeFilter);
}

void AlbumIconViewFilter::slotTextFilterChanged(const QString& text)
{
    AlbumLister::instance()->setTextFilter(text);
}

void AlbumIconViewFilter::slotItemsFilterMatch(bool match)
{
    QStringList filtersList;
    QString     message;

    if (!d->textFilter->text().isEmpty())
        filtersList.append(i18n("<br><nobr><i>Text</i></nobr>"));

    if (d->mimeFilter->mimeFilter() != MimeFilter::AllFiles)
        filtersList.append(i18n("<br><nobr><i>Mime Type</i></nobr>"));

    if (d->ratingFilter->rating() != 0 || d->ratingFilter->ratingFilterCondition() != AlbumLister::GreaterEqualCondition)
        filtersList.append(i18n("<br/><nobr><i>Rating</i></nobr>"));

    if (AlbumLister::instance()->tagFiltersIsActive())
        filtersList.append(i18n("<br><nobr><i>Tags</i></nobr>"));

    if (filtersList.count() > 1)
        message = i18n("<nobr><b>Active filters:</b></nobr>");
    else
        message = i18n("<nobr><b>Active filter:</b></nobr>");

    message.append(filtersList.join(QString()));

    if (filtersList.isEmpty())
    {
        QToolTip::add(d->led, i18n("No active filter"));
        d->led->setLedColor(StatusLed::Gray);
    }
    else
    {
        QToolTip::add(d->led, message);
        d->led->setLedColor(match ? StatusLed::Green : StatusLed::Red);
    }
}

bool AlbumIconViewFilter::eventFilter(QObject *object, QEvent *e)
{
    QWidget *widget = static_cast<QWidget*>(object);

    if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        if ( widget->rect().contains(event->pos()) && d->led->ledColor() != StatusLed::Gray)
        {
            // Reset all filters settings.
            d->textFilter->setText(QString());
            d->ratingFilter->setRating(0);
            d->ratingFilter->setRatingFilterCondition(AlbumLister::GreaterEqualCondition);
            d->mimeFilter->setMimeFilter(MimeFilter::AllFiles);
            emit signalResetTagFilters();
        }
    }

    return false;
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kled.h>

// Local includes.

#include "ddebug.h"
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
        tagFiltersActive = false;
        textFilter       = 0;
        mimeFilter       = 0;
        ratingFilter     = 0;
        led              = 0;
        activeFilters    = QColor(255, 192, 0);
        inactiveFilters  = Qt::green;
    }

    bool           tagFiltersActive;

    QColor         activeFilters;
    QColor         inactiveFilters;

    QLineEdit     *textFilter;
//    SearchTextBar *textFilter;

    KLed          *led;

    MimeFilter    *mimeFilter;

    RatingFilter  *ratingFilter;
};

AlbumIconViewFilter::AlbumIconViewFilter(QWidget* parent)
                   : KHBox(parent)
{
    d = new AlbumIconViewFilterPriv;

    d->led = new KLed(this);
    d->led->setMinimumSize(parent->height()-2, parent->height()-2);
    d->led->installEventFilter(this);
    d->led->setState(KLed::On);
    d->led->setWhatsThis(i18n("This light indicate if at least one filter is active over icon view. "
                              "This include all filters from status bar and all Tag Filters from right sidebar. "
                              "If this light is orange, filter is active. "
                              "If this light is green, nothing is filtered. "
                              "Click over with right mouse button to reset all filters."));

//    d->textFilter = new SearchTextBar(this);
    d->textFilter = new QLineEdit(this);
    d->textFilter->setToolTip(i18n("Text quick filter (search)"));
    d->textFilter->setWhatsThis(i18n("Here you can enter search patterns to quickly "
                                     "filter this view on file names, captions "
                                     "(comments), and tags"));

    d->mimeFilter   = new MimeFilter(this);
    d->ratingFilter = new RatingFilter(this);

    setSpacing(KDialog::spacingHint());
    setMargin(0);

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int, AlbumLister::RatingCondition)),
            this, SLOT(slotRatingFilterChanged(int, AlbumLister::RatingCondition)));

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SLOT(slotMimeTypeFilterChanged(int)));

    connect(d->textFilter, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextFilterChanged(const QString&)));

    connect(AlbumLister::instance(), SIGNAL(signalItemsTextFilterMatch(bool)),
            d->textFilter, SLOT(slotSearchResult(bool)));
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
    d->ratingFilter->setEnabled(settings->getIconShowRating());
    d->textFilter->setEnabled(settings->getIconShowName()     ||
                              settings->getIconShowComments() ||
                              settings->getIconShowTags());
    // NOTE: Mime Type filter is always enable.
}

void AlbumIconViewFilter::saveSettings()
{
    AlbumSettings::instance()->setRatingFilterCond(d->ratingFilter->ratingFilterCondition());
}

void AlbumIconViewFilter::slotRatingFilterChanged(int rating, AlbumLister::RatingCondition cond)
{
    checkForLed();
    AlbumLister::instance()->setRatingFilter(rating, cond);
}

void AlbumIconViewFilter::slotMimeTypeFilterChanged(int mimeTypeFilter)
{
    checkForLed();
    AlbumLister::instance()->setMimeTypeFilter(mimeTypeFilter);
}

void AlbumIconViewFilter::slotTextFilterChanged(const QString& text)
{
    checkForLed();
    AlbumLister::instance()->setTextFilter(text);
}


void AlbumIconViewFilter::slotTagFiltersChanged(bool isActive)
{
    d->tagFiltersActive = isActive;
    checkForLed();
}

void AlbumIconViewFilter::checkForLed()
{
    QColor ledColor = d->inactiveFilters;
    QStringList filtersList;
    QString     message;

    if (!d->textFilter->text().isEmpty())
    {
        ledColor = d->activeFilters;
        filtersList.append(i18n("<br><nobr><i>Text</i></nobr>"));
    }

    if (d->mimeFilter->mimeFilter() != MimeFilter::AllFiles)
    {
        ledColor = d->activeFilters;
        filtersList.append(i18n("<br><nobr><i>Mime Type</i></nobr>"));
    }

    if (d->ratingFilter->rating() != 0)
    {
        ledColor = d->activeFilters;
        filtersList.append(i18n("<br><nobr><i>Rating</i></nobr>"));
    }

    if (d->tagFiltersActive)
    {
        ledColor = d->activeFilters;
        filtersList.append(i18n("<br><nobr><i>Tags</i></nobr>"));
    }

    d->led->setColor(ledColor);

    if (filtersList.count() > 1) 
        message = i18n("<nobr><b>Active filters:</b></nobr>");
    else
        message = i18n("<nobr><b>Active filter:</b></nobr>");

    message.append(filtersList.join(QString()));

    if (ledColor == d->inactiveFilters)
        d->led->setToolTip(i18n("No active filter"));
    else
        d->led->setToolTip(message);
}

bool AlbumIconViewFilter::eventFilter(QObject *object, QEvent *e) 
{
    QWidget *widget = static_cast<QWidget*>(object);

    if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        if ( widget->rect().contains(event->pos()) && d->led->color() == d->activeFilters)
        {
            // Reset all filters settings.
            d->textFilter->setText(QString());
            d->ratingFilter->setRating(0);
            d->ratingFilter->setRatingFilterCondition(AlbumLister::GreaterEqualCondition);
            d->mimeFilter->setMimeFilter(MimeFilter::AllFiles);
            emit signalResetTagFilters();
            // No need to call checkForLed() here, it will be done from slotTagFiltersChanged().
        }
    }

    return false;
}

}  // namespace Digikam

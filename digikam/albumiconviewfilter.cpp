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

#include "albumiconviewfilter.h"
#include "albumiconviewfilter.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kdialog.h>

// Local includes

#include "albumsettings.h"
#include "ratingfilter.h"
#include "mimefilter.h"
#include "statusled.h"

namespace Digikam
{

class AlbumIconViewFilterPriv
{
public:

    AlbumIconViewFilterPriv()
    {
        textFilter   = 0;
        mimeFilter   = 0;
        ratingFilter = 0;
        led          = 0;
    }

/* NOTE: There is a problem with Qt4.3 and KDE < 4.1.0 if statusbar host a KLineEdit:
         digiKam crash. Text Filter bar is replaced by a simple QLineEdit in this case.
 */
#if KDE_IS_VERSION(4,1,0)
    SearchTextBar       *textFilter;
#else
    QLineEdit           *textFilter;
#endif

    StatusLed           *led;

    MimeFilter          *mimeFilter;

    RatingFilter        *ratingFilter;

    ImageFilterSettings  settings;
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

#if KDE_IS_VERSION(4,1,0)
    d->textFilter = new SearchTextBar(this, "AlbumIconViewFilterSearchTextBar");
    d->textFilter->setTextQueryCompletion(true);
#else
    d->textFilter = new QLineEdit(this);
#endif

    d->textFilter->setToolTip(i18n("Text quick filter (search)"));
    d->textFilter->setWhatsThis(i18n("Enter search patterns to quickly filter this view on "
                                     "file names, captions (comments), and tags"));

    d->mimeFilter   = new MimeFilter(this);
    d->ratingFilter = new RatingFilter(this);

    layout()->setAlignment(d->ratingFilter, Qt::AlignCenter);
    setSpacing(KDialog::spacingHint());
    setMargin(0);

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int, ImageFilterSettings::RatingCondition)),
            this, SIGNAL(ratingFilterChanged(int, ImageFilterSettings::RatingCondition)));

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SIGNAL(mimeTypeFilterChanged(int)));

    connect(d->textFilter, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SIGNAL(textFilterChanged(const SearchTextSettings&)));
}

AlbumIconViewFilter::~AlbumIconViewFilter()
{
    delete d;
}

void AlbumIconViewFilter::readSettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    d->ratingFilter->setRatingFilterCondition((Digikam::ImageFilterSettings::RatingCondition)
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

void AlbumIconViewFilter::slotFilterMatches(bool match)
{
    QStringList filtersList;
    QString     message;

    if (!d->textFilter->text().isEmpty())
        filtersList.append(i18n("<br/><nobr><i>Text</i></nobr>"));

    if (d->mimeFilter->mimeFilter() != MimeFilter::AllFiles)
        filtersList.append(i18n("<br/><nobr><i>Mime Type</i></nobr>"));

    if (d->ratingFilter->rating() != 0 || d->ratingFilter->ratingFilterCondition() != ImageFilterSettings::GreaterEqualCondition)
        filtersList.append(i18n("<br/><nobr><i>Rating</i></nobr>"));

    if (d->settings.isFilteringByTags())
        filtersList.append(i18n("<br/><nobr><i>Tags</i></nobr>"));

    if (filtersList.count() > 1)
        message = i18n("<nobr><b>Active filters:</b></nobr>");
    else
        message = i18n("<nobr><b>Active filter:</b></nobr>");

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

void AlbumIconViewFilter::slotFilterMatchesForText(bool match)
{
#if KDE_IS_VERSION(4,1,0)
    d->textFilter->slotSearchResult(match);
#endif
}

void AlbumIconViewFilter::slotFilterSettingsChanged(const ImageFilterSettings &settings)
{
    d->settings = settings;
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
            d->ratingFilter->setRatingFilterCondition(ImageFilterSettings::GreaterEqualCondition);
            d->mimeFilter->setMimeFilter(MimeFilter::AllFiles);
            emit resetTagFilters();
        }
    }

    return false;
}

}  // namespace Digikam

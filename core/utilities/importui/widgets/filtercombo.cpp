/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Filter combobox
 *
 * Copyright (C) 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
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

#include "filtercombo.h"

// Qt includes

#include <QStringList>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "importfilters.h"
#include "camiteminfo.h"

namespace Digikam
{

// JVC camera (see bug #133185).
const QString FilterComboBox::defaultIgnoreNames(QLatin1String("mgr_data pgr_mgr"));
// HP Photosmart camera (see bug #156338).
// Minolta camera in PTP mode
const QString FilterComboBox::defaultIgnoreExtensions(QLatin1String("dsp dps"));

// ---------------------------------------------------------------------------------

class FilterComboBox::Private
{
public:

    Private()
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("Import Filters"));

        for (int i = 0; true; ++i)
        {
            QString filter = group.readEntry(QString::fromUtf8("Filter%1").arg(i), QString());

            if (filter.isEmpty())
            {
                break;
            }

            Filter* const f = new Filter;
            f->fromString(filter);
            filters.append(f);
        }

        FilterComboBox::defaultFilters(&filters);
        currentFilter = group.readEntry(QLatin1String("CurrentFilter"), 0);
    }

    ~Private()
    {
        qDeleteAll(filters);
    }

    int                         currentFilter;
    QList<Filter*>              filters;
    QHash<QString, QRegExp>     filterHash;
    QHash<QString, QStringList> mimeHash;
};

FilterComboBox::FilterComboBox(QWidget* const parent)
    : QComboBox(parent),
      d(new Private)
{
    fillCombo();

    connect(this, SIGNAL(activated(int)),
            this, SLOT(indexChanged(int)));
}

FilterComboBox::~FilterComboBox()
{
    delete d;
}

Filter* FilterComboBox::currentFilter()
{
    Filter* const filter = d->filters.value(d->currentFilter);
    return filter;
}

void FilterComboBox::defaultFilters(FilterList* const filters)
{
    if (filters->count() == 0)
    {
        Filter*       f = new Filter;
        f->name         = i18nc("@item:inlistbox", "All Files");
        filters->append(f);

        f               = new Filter;
        f->name         = i18nc("@item:inlistbox", "Only New Files");
        f->onlyNew      = true;
        filters->append(f);

        f               = new Filter;
        f->name         = i18nc("@item:inlistbox", "Raw Files");
        f->mimeFilter   = QLatin1String("image/x-nikon-nef;image/x-fuji-raf;image/x-adobe-dng;"
                          "image/x-panasonic-raw;image/x-olympus-orf;image/x-kodak-dcr;"
                          "image/x-kodak-k25;image/x-sony-arw;image/x-minolta-mrw;"
                          "image/x-kodak-kdc;image/x-sigma-x3f;image/x-sony-srf;"
                          "image/x-pentax-pef;image/x-panasonic-raw2;image/x-canon-crw;"
                          "image/x-sony-sr2;image/x-canon-cr2");
        filters->append(f);

        f               = new Filter;
        f->name         = i18nc("@item:inlistbox", "JPG/TIFF Files");
        f->mimeFilter   = QLatin1String("image/jpeg;image/tiff");
        filters->append(f);

        f               = new Filter;
        f->name         = i18nc("@item:inlistbox", "Video Files");
        f->mimeFilter   = QLatin1String("video/quicktime;video/mp4;video/x-msvideo;video/mpeg");
        filters->append(f);
    }
}

void FilterComboBox::fillCombo()
{
    clear();

    foreach(Filter* const f, d->filters)
    {
        addItem(f->name);
    }

    setCurrentIndex(d->currentFilter);
}

void FilterComboBox::indexChanged(int index)
{
    if (index != d->currentFilter)
    {
        d->currentFilter = index;
        Filter* const filter = d->filters.value(d->currentFilter);
        emit filterChanged(filter);
    }
}

void FilterComboBox::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Import Filters"));

    group.writeEntry(QLatin1String("CurrentFilter"), d->currentFilter);
}

}  // namespace Digikam

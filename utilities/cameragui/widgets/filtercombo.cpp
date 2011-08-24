/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Filter combobox
 *
 * Copyright (C) 2010-2011 by Petri Damstén <petri.damsten@iki.fi>
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

#include "filtercombo.moc"

// Qt includes

#include <QStringList>

// KDE includes

#include <KLocale>
#include <KDebug>
#include <KGlobal>
#include <KConfigGroup>
#include <KMimeType>

// Local includes

#include "filtercombo.h"
#include "importfilters.h"
#include "cameraiconitem.h"
#include "camiteminfo.h"

namespace Digikam
{

Filter::Filter()
    : onlyNew(false)
{
}

QString Filter::toString()
{
    return QString("%1|%2|%3|%4|%5")
                   .arg(name)
                   .arg(onlyNew ? "true" : "false")
                   .arg(fileFilter.join(";"))
                   .arg(pathFilter.join(";"))
                   .arg(mimeFilter);
}

void Filter::fromString(const QString& filter)
{
    QStringList s = filter.split('|');
    name          = s.value(0);
    onlyNew       = (s.value(1) == "true");

    if (!s.value(2).isEmpty())
    {
        fileFilter = s.value(2).split(';');
    }
    if (!s.value(3).isEmpty())
    {
        pathFilter = s.value(3).split(';');
    }
    if (!s.value(4).isEmpty())
    {
        mimeFilter = s.value(4);
    }
}

// ---------------------------------------------------------------------------------

class FilterComboBox::FilterComboBoxPriv
{
public:

    FilterComboBoxPriv()
    {
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group("Import Filters");

        for (int i = 0; true; ++i)
        {
            QString filter = group.readEntry(QString("Filter%1").arg(i), QString());

            if (filter.isEmpty())
            {
                break;
            }

            Filter* f = new Filter;
            f->fromString(filter);
            filters.append(f);
        }

        FilterComboBox::defaultFilters(&filters);
        currentFilter = group.readEntry("CurrentFilter", 0);
    }

    ~FilterComboBoxPriv()
    {
        qDeleteAll(filters);
    }

    int            currentFilter;
    QList<Filter*> filters;
    QHash<QString, QRegExp>     filterHash;
    QHash<QString, QStringList> mimeHash;
};

FilterComboBox::FilterComboBox(QWidget* const parent)
    : KComboBox(parent), d(new FilterComboBoxPriv)
{
    connect(this, SIGNAL(activated(int)), this, SLOT(indexChanged(int)));
    fillCombo();
}

FilterComboBox::~FilterComboBox()
{
}

void FilterComboBox::defaultFilters(FilterList* filters)
{
    if (filters->count() == 0)
    {
        Filter* f     = new Filter;
        f->name       = i18n("All Files");
        filters->append(f);

        f             = new Filter;
        f->name       = i18n("Only New Files");
        f->onlyNew    = true;
        filters->append(f);

        f             = new Filter;
        f->name       = i18n("Raw Files");
        f->mimeFilter = "image/x-nikon-nef;image/x-fuji-raf;image/x-adobe-dng;"
                        "image/x-panasonic-raw;image/x-olympus-orf;image/x-kodak-dcr;"
                        "image/x-kodak-k25;image/x-sony-arw;image/x-minolta-mrw;"
                        "image/x-kodak-kdc;image/x-sigma-x3f;image/x-sony-srf;"
                        "image/x-pentax-pef;image/x-panasonic-raw2;image/x-canon-crw;"
                        "image/x-sony-sr2;image/x-canon-cr2";
        filters->append(f);

        f             = new Filter;
        f->name       = i18n("JPG/TIFF Files");
        f->mimeFilter = "image/jpeg;image/tiff";
        filters->append(f);

        f             = new Filter;
        f->name       = i18n("Video Files");
        f->mimeFilter = "video/quicktime;video/mp4;video/x-msvideo;video/mpeg";
        filters->append(f);
    }
}

void FilterComboBox::fillCombo()
{
    while (count() > 0)
    {
        removeItem(0);
    }

    foreach (Filter* f, d->filters)
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
        emit filterChanged();
    }
}

void FilterComboBox::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Import Filters");

    group.writeEntry("CurrentFilter", d->currentFilter);
}

const QRegExp& FilterComboBox::regexp(const QString& wildcard)
{
   if (!d->filterHash.contains(wildcard))
   {
      QRegExp rx(wildcard.toLower());
      rx.setPatternSyntax(QRegExp::Wildcard);
      d->filterHash[wildcard] = rx;
   }

   return d->filterHash[wildcard];
}

bool FilterComboBox::match(const QStringList& wildcards, const QString& name)
{
    bool match = false;

    foreach (const QString& wildcard, wildcards)
    {
        match = regexp(wildcard).exactMatch(name);
        //kDebug() << "**" << wildcard << name << match;
        if (match)
        {
            break;
        }
    }

    return match;
}

const QStringList& FilterComboBox::mimeWildcards(const QString& mime)
{
    if (!d->mimeHash.contains(mime))
    {
        QStringList& wc  = d->mimeHash[mime];
        QStringList list = mime.split(';');
        foreach (const QString& m, list)
        {
            KMimeType::Ptr mime = KMimeType::mimeType(m);
            foreach (const QString& pattern, mime->patterns())
            {
                wc.append(pattern);
            }
        }
    }

    return d->mimeHash[mime];
}

bool FilterComboBox::matchesCurrentFilter(const CamItemInfo& item)
{
    //kDebug() << item.downloaded << item.folder << item.name;

    Filter* currentFilter = d->filters.value(d->currentFilter);
    if (!currentFilter)
    {
        return true;
    }

    if (currentFilter->onlyNew)
    {
        if (item.downloaded == CamItemInfo::DownloadedYes)
        {
            return false;
        }
    }

    QString folder = item.folder.toLower();
    QString name   = item.name.toLower();

    if (!currentFilter->fileFilter.isEmpty())
    {
        if (!match(currentFilter->fileFilter, name))
        {
            return false;
        }
    }

    if (!currentFilter->pathFilter.isEmpty())
    {
        if (!match(currentFilter->pathFilter, folder))
        {
            return false;
        }
    }

    if (!currentFilter->mimeFilter.isEmpty())
    {
        if (!match(mimeWildcards(currentFilter->mimeFilter), name))
        {
            return false;
        }
    }

    return true;
}

}  // namespace Digikam

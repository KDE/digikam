/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Config panel for BQM dplugins.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginbqmsetup.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginbqmconfview.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginBqmSetup::Private
{
public:

    explicit Private()
      : pluginsNumber(0),
        hbox(0),
        pluginFilter(0),
        pluginsList(0)
    {
    }

    QLabel*             pluginsNumber;

    QWidget*            hbox;

    SearchTextBar*      pluginFilter;
    DPluginBqmConfView* pluginsList;
};

DPluginBqmSetup::DPluginBqmSetup(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid   = new QGridLayout(this);

    d->pluginFilter           = new SearchTextBar(this, QLatin1String("PluginsBqmSearchBar"));
    d->pluginsNumber          = new QLabel(this);

    d->pluginsList            = new DPluginBqmConfView(this);
    QStringList labels;
    labels.append(i18n("Name"));
    labels.append(i18n("Categories"));
    labels.append(i18n("Tools"));
    labels.append(i18n("Description"));
    labels.append(i18n("Authors"));
    d->pluginsList->setHeaderLabels(labels);

    grid->addWidget(d->pluginFilter,           0, 0, 1, 1);
    grid->addWidget(d->pluginsNumber,          0, 1, 1, 1);
    grid->addWidget(d->pluginsList,            1, 0, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->pluginsList, SIGNAL(signalSearchResult(int)),
            this, SLOT(slotSearchResult(int)));

    connect(d->pluginFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    // --------------------------------------------------------

    updateInfo();
}

DPluginBqmSetup::~DPluginBqmSetup()
{
    delete d;
}

void DPluginBqmSetup::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->pluginsList->setFilter(settings.text, settings.caseSensitive);
}

void DPluginBqmSetup::updateInfo()
{
    if (d->pluginFilter->text().isEmpty())
    {
        // List is not filtered
        int cnt = d->pluginsList->count();

        if (cnt > 0)
            d->pluginsNumber->setText(i18np("1 plugin installed", "%1 plugins installed", cnt));
        else
            d->pluginsNumber->setText(i18n("No plugin installed"));
    }
    else
    {
        // List filtering is active
        int cnt = d->pluginsList->visible();

        if (cnt > 0)
            d->pluginsNumber->setText(i18np("1 plugin found", "%1 plugins found", cnt));
        else
            d->pluginsNumber->setText(i18n("No plugin found"));
    }
}

void DPluginBqmSetup::slotSetFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->pluginsList->setFilter(filter, cs);
    updateInfo();
}

void DPluginBqmSetup::slotSearchResult(int found)
{
    d->pluginFilter->slotSearchResult(found ? true : false);
}

}  // namespace Digikam

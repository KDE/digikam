/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Config panel for generic dplugins.
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

#include "dpluginsetup.h"

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

#include "dpluginconfview.h"
#include "dpluginaboutdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginSetup::Private
{
public:

    explicit Private()
      : pluginsNumber(0),
        pluginsNumberActivated(0),
        checkAllBtn(0),
        clearBtn(0),
        hbox(0),
        pluginFilter(0),
        pluginsList(0)
    {
    }

    QLabel*                 pluginsNumber;
    QLabel*                 pluginsNumberActivated;

    QPushButton*            checkAllBtn;
    QPushButton*            clearBtn;

    QWidget*                hbox;

    SearchTextBar*          pluginFilter;
    DPluginConfView*        pluginsList;
};

DPluginSetup::DPluginSetup(DPluginAction::ActionType type, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid   = new QGridLayout(this);

    d->pluginFilter           = new SearchTextBar(this, QLatin1String("PluginsSearchBar"));
    d->pluginsNumber          = new QLabel(this);
    d->pluginsNumberActivated = new QLabel(this);
    d->checkAllBtn            = new QPushButton(i18n("Check All"), this);
    d->clearBtn               = new QPushButton(i18n("Clear"),     this);

    d->pluginsList            = new DPluginConfView(type, this);
    QStringList labels;
    labels.append(i18n("Name"));
    labels.append(i18n("Categories"));
    labels.append(i18n("Tools"));
    labels.append(i18n("Description"));
    labels.append(i18n("Authors"));
    d->pluginsList->setHeaderLabels(labels);

    grid->addWidget(d->pluginFilter,           0, 0, 1, 1);
    grid->addWidget(d->pluginsNumber,          0, 1, 1, 1);
    grid->addWidget(d->pluginsNumberActivated, 0, 2, 1, 1);
    grid->addWidget(d->checkAllBtn,            0, 4, 1, 1);
    grid->addWidget(d->clearBtn,               0, 5, 1, 1);
    grid->addWidget(d->pluginsList,            1, 0, 1, 6);
    grid->setColumnStretch(3, 10);
    grid->setRowStretch(1, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->checkAllBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckAll()));

    connect(d->clearBtn, SIGNAL(clicked()),
            this, SLOT(slotClearList()));

    connect(d->pluginsList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotAboutPlugin(QTreeWidgetItem*)));

    connect(d->pluginsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotAboutPlugin(QTreeWidgetItem*)));

    connect(d->pluginsList, SIGNAL(signalSearchResult(int)),
            this, SLOT(slotSearchResult(int)));

    connect(d->pluginFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    // --------------------------------------------------------

    updateInfo();
}

DPluginSetup::~DPluginSetup()
{
    delete d;
}

void DPluginSetup::applySettings()
{
    d->pluginsList->apply();
}

void DPluginSetup::slotAboutPlugin(QTreeWidgetItem* item)
{
    if (!item) return;

    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(d->pluginsList->plugin(item));
    dlg->exec();
    delete dlg;
}

void DPluginSetup::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->pluginsList->setFilter(settings.text, settings.caseSensitive);
}

void DPluginSetup::updateInfo()
{
    if (d->pluginFilter->text().isEmpty())
    {
        // List is not filtered
        int cnt = d->pluginsList->count();

        if (cnt > 0)
            d->pluginsNumber->setText(i18np("1 plugin installed", "%1 plugins installed", cnt));
        else
            d->pluginsNumber->setText(i18n("No plugin installed"));

        int act = d->pluginsList->actived();

        if (act > 0)
            d->pluginsNumberActivated->setText(i18ncp("%1: number of plugins activated", "(%1 activated)", "(%1 activated)", act));
        else
            d->pluginsNumberActivated->setText(QString());
    }
    else
    {
        // List filtering is active
        int cnt = d->pluginsList->visible();

        if (cnt > 0)
            d->pluginsNumber->setText(i18np("1 plugin found", "%1 plugins found", cnt));
        else
            d->pluginsNumber->setText(i18n("No plugin found"));

        d->pluginsNumberActivated->setText(QString());
    }
}

void DPluginSetup::slotCheckAll()
{
    d->pluginsList->selectAll();
    updateInfo();
}

void DPluginSetup::slotClearList()
{
    d->pluginsList->clearAll();
    updateInfo();
}

void DPluginSetup::slotItemClicked()
{
    updateInfo();
}

void DPluginSetup::slotSetFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->pluginsList->setFilter(filter, cs);
    updateInfo();
}

void DPluginSetup::slotSearchResult(int found)
{
    d->pluginFilter->slotSearchResult(found ? true : false);
}

}  // namespace Digikam

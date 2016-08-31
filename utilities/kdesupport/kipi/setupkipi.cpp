/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup kipi tools.
 *
 * Copyright (C) 2007-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupkipi.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QBuffer>
#include <QStandardPaths>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Libkipi includes

#include <KIPI/PluginLoader>
#include <KIPI/ConfigWidget>
#include <libkipi_version.h>

using namespace KIPI;

namespace Digikam
{

class SetupKipi::Private
{
public:


    Private() :
        pluginsNumber(0),
        pluginsNumberActivated(0),
        kipipluginsVersion(0),
        libkipiVersion(0),
        checkAllBtn(0),
        clearBtn(0),
        grid(0),
        hbox(0),
        kipiLogoLabel(0),
        pluginFilter(0),
        pluginsList(0)
    {
    }

    QLabel*         pluginsNumber;
    QLabel*         pluginsNumberActivated;
    QLabel*         kipipluginsVersion;
    QLabel*         libkipiVersion;

    QPushButton*    checkAllBtn;
    QPushButton*    clearBtn;

    QGridLayout*    grid;

    QWidget*        hbox;
    QLabel*         kipiLogoLabel;

    SearchTextBar*  pluginFilter;
    ConfigWidget*   pluginsList;
};

SetupKipi::SetupKipi(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel        = new QWidget(viewport());
    d->grid                     = new QGridLayout(panel);

    d->pluginFilter             = new SearchTextBar(panel, QLatin1String("PluginsSearchBar"));
    d->pluginsNumber            = new QLabel(panel);
    d->pluginsNumberActivated   = new QLabel(panel);
    d->hbox                     = new QWidget(panel);
    QHBoxLayout* const hboxLay  = new QHBoxLayout(d->hbox);
    d->checkAllBtn              = new QPushButton(i18n("Check All"), d->hbox);
    d->clearBtn                 = new QPushButton(i18n("Clear"),     d->hbox);
    QWidget* const space        = new QWidget(d->hbox);

    hboxLay->addWidget(d->checkAllBtn);
    hboxLay->addWidget(d->clearBtn);
    hboxLay->addWidget(space);
    hboxLay->setStretchFactor(space, 10);

    PluginLoader* const loader  = PluginLoader::instance();
    d->kipipluginsVersion       = new QLabel(QString::fromLatin1("Kipi-plugins: %1").arg(loader ? loader->kipiPluginsVersion() : QString()), panel);
    d->libkipiVersion           = new QLabel(QString::fromLatin1("Libkipi: %1").arg(QLatin1String(KIPI_VERSION_STRING)), panel);
    d->libkipiVersion->setAlignment(Qt::AlignRight);

    d->pluginsList              = new ConfigWidget(panel);
    QStringList labels;
    labels.append(i18n("Name"));
    labels.append(i18n("Categories"));
    labels.append(i18n("Description"));
    labels.append(i18n("Author"));
    d->pluginsList->setHeaderLabels(labels);

    d->kipiLogoLabel = new QLabel(panel);
    d->kipiLogoLabel->setFocusPolicy(Qt::NoFocus);
    d->kipiLogoLabel->setTextFormat(Qt::RichText);
    d->kipiLogoLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    d->kipiLogoLabel->setOpenExternalLinks(true);
    QFontMetrics fm(d->kipipluginsVersion->font());
    QRect r          = fm.boundingRect(QString::fromLatin1("XX"));
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    QPixmap img(QLatin1String(":/images/kipi-plugins_logo.png"));
    img = img.scaledToHeight(r.height()*3, Qt::SmoothTransformation);
    img.save(&buffer, "PNG");
    d->kipiLogoLabel->setText(QString::fromLatin1("<a href=\"https://projects.kde.org/projects/extragear/graphics/kipi-plugins\">%1</a>")
                              .arg(QString::fromLatin1("<img src=\"data:image/png;base64,%1\">")
                              .arg(QString::fromLatin1(byteArray.toBase64().data()))));

    d->grid->addWidget(d->pluginFilter,           0, 0, 1, 1);
    d->grid->addWidget(d->pluginsNumber,          0, 1, 1, 1);
    d->grid->addWidget(d->pluginsNumberActivated, 0, 2, 1, 1);
    d->grid->addWidget(d->kipipluginsVersion,     0, 4, 1, 1);
    d->grid->addWidget(d->libkipiVersion,         1, 4, 1, 1);
    d->grid->addWidget(d->hbox,                   1, 0, 1, 2);
    d->grid->addWidget(d->kipiLogoLabel,          0, 6, 2, 1);
    d->grid->addWidget(d->pluginsList,            2, 0, 1, -1);
    d->grid->setColumnStretch(3, 10);
    d->grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->grid->setSpacing(spacing);

    // --------------------------------------------------------

    setWidget(panel);
    setWidgetResizable(true);
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);

    // --------------------------------------------------------

    connect(d->checkAllBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckAll()));

    connect(d->clearBtn, SIGNAL(clicked()),
            this, SLOT(slotClearList()));

    connect(d->pluginsList, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotItemClicked()));

    connect(d->pluginsList, SIGNAL(signalSearchResult(int)),
            this, SLOT(slotSearchResult(int)));

    connect(d->pluginFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    // --------------------------------------------------------

    updateInfo();
}

SetupKipi::~SetupKipi()
{
    delete d;
}

void SetupKipi::applySettings()
{
    d->pluginsList->apply();
}

void SetupKipi::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->pluginsList->setFilter(settings.text, settings.caseSensitive);
}

void SetupKipi::updateInfo()
{
    if (d->pluginFilter->text().isEmpty())
    {
        // List is not filtered
        int cnt = d->pluginsList->count();

        if (cnt > 0)
            d->pluginsNumber->setText(i18np("1 Kipi plugin installed", "%1 Kipi plugins installed", cnt));
        else
            d->pluginsNumber->setText(i18n("No Kipi plugin installed"));

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
            d->pluginsNumber->setText(i18np("1 Kipi plugin found", "%1 Kipi plugins found", cnt));
        else
            d->pluginsNumber->setText(i18n("No Kipi plugin found"));

        d->pluginsNumberActivated->setText(QString());
    }
}

void SetupKipi::slotCheckAll()
{
    d->pluginsList->selectAll();
    updateInfo();
}

void SetupKipi::slotClearList()
{
    d->pluginsList->clearAll();
    updateInfo();
}

void SetupKipi::slotItemClicked()
{
    updateInfo();
}

void SetupKipi::slotSetFilter(const QString& filter, Qt::CaseSensitivity cs)
{
    d->pluginsList->setFilter(filter, cs);
    updateInfo();
}

void SetupKipi::slotSearchResult(int found)
{
    d->pluginFilter->slotSearchResult(found ? true : false);
}

}  // namespace Digikam

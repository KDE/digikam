/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : a bar to indicate pending metadata
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "metadatastatusbar.h"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QDebug>

// KDE includes

#include <klocalizedstring.h>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

#include "metadatasettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class MetadataStatusBar::Private
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
        applyBtn    = 0;
    }

    int               status;

    RAdjustableLabel* info;
    QToolButton*      applyBtn;

};

MetadataStatusBar::MetadataStatusBar(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    QHBoxLayout* const vlay = new QHBoxLayout(this);

    d->info        = new RAdjustableLabel(this);
    d->info->setContextMenuPolicy(Qt::NoContextMenu);
    d->info->setAutoFillBackground(true);
    d->info->setFocusPolicy(Qt::NoFocus);
    d->info->setAdjustedText(i18n("No pending metadata synchronization"));

    d->info->setWhatsThis(i18n("If lazy syncronization is enabled in metadatasettings,"
                               "The status bar will display the number of items waiting for syncronization"));

    d->applyBtn    = new QToolButton(this);
    d->applyBtn->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->applyBtn->setToolTip(i18n("Apply pending changes to metadata"));
    d->applyBtn->setFocusPolicy(Qt::NoFocus);
    d->applyBtn->setAutoRaise(true);
    d->applyBtn->setDisabled(true);

    vlay->addWidget(d->info);
    vlay->addWidget(d->applyBtn);
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->setMargin(0);
    vlay->setStretchFactor(d->info, 10);

    connect(MetadataSettings::instance(), SIGNAL(settingsChanged()), this,
            SLOT(slotSettingsChanged()));
    connect(d->applyBtn, SIGNAL(released()),
            MetadataHubMngr::instance(), SLOT(slotApplyPending()));

    connect(MetadataHubMngr::instance(), SIGNAL(signalPendingMetadata(int)),
            this, SLOT(setPendingItems(int)));

    if(MetadataSettings::instance()->settings().useLazySync)
        this->show();
    else
        this->hide();
}

MetadataStatusBar::~MetadataStatusBar()
{
    delete d;
}

//void MetadataStatusBar::slotFilterMatches(bool match)
//{
//    QStringList filtersList;
//    QString     message;

//    if (d->settings.isFilteringByText())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Text</i></nobr>"));
//    }

//    if (d->settings.isFilteringByTypeMime())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Mime Type</i></nobr>"));
//    }

//    if (d->settings.isFilteringByGeolocation())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Geolocation</i></nobr>"));
//    }

//    if (d->settings.isFilteringByRating())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Rating</i></nobr>"));
//    }

//    if (d->settings.isFilteringByPickLabels())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Pick Labels</i></nobr>"));
//    }

//    if (d->settings.isFilteringByColorLabels())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Color Labels</i></nobr>"));
//    }

//    if (d->settings.isFilteringByTags())
//    {
//        filtersList.append(i18n("<br/><nobr><i>Tags</i></nobr>"));
//    }

//    if (filtersList.count() > 1)
//    {
//        message = i18n("<nobr><b>Active filters:</b></nobr>");
//    }
//    else
//    {
//        message = i18n("<nobr><b>Active filter:</b></nobr>");
//    }

//    message.append(filtersList.join(QString()));

//    if (filtersList.isEmpty())
//    {
//        d->info->setAdjustedText(i18n("No active filter"));
//        d->info->setToolTip(QString());
//        d->resetBtn->setEnabled(false);
//        d->status = Private::None;
//    }
//    else
//    {
//        if (filtersList.count() == 1)
//        {
//            d->info->setAdjustedText(i18n("One active filter"));
//        }
//        else
//        {
//            d->info->setAdjustedText(i18np("1 active filter", "%1 active filters", filtersList.count()));
//        }

//        d->info->setToolTip(message);
//        d->resetBtn->setEnabled(true);
//        d->status = match ? Private::Match : Private::NotMatch;
//    }

//    QPalette pal = palette();

//    switch(d->status)
//    {
//        case Private::NotMatch:
//            pal.setColor(backgroundRole(), QColor(255, 200, 200));
//            pal.setColor(foregroundRole(), Qt::black);
//            break;
//        case Private::Match:
//            pal.setColor(backgroundRole(), QColor(200, 255, 200));
//            pal.setColor(foregroundRole(), Qt::black);
//            break;
//        default: // Private::None
//            break;
//    }

//    d->info->setPalette(pal);

//    update();
//}

void MetadataStatusBar::slotSettingsChanged()
{
    qDebug() << "Settings changed";
    if(MetadataSettings::instance()->settings().useLazySync)
        this->show();
    else
        this->hide();
}

void MetadataStatusBar::slotSetPendingItems(int number)
{
    if(number == 0)
    {
        d->info->setAdjustedText(i18n("No pending metadata synchronization"));
        d->applyBtn->setDisabled(true);
    }
    else
    {
        d->info->setAdjustedText(i18np("1 file awaits syncronization",
                                       "%l files await syncronization",
                                       number));
        d->applyBtn->setDisabled(false);
    }
}

}  // namespace Digikam


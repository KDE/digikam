/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server control widget.
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmediaserverctrl.h"

// Qt includes

#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmediaservermngr.h"
#include "workingwidget.h"

namespace Digikam
{

class DMediaServerCtrl::Private
{
public:

    Private() :
        mngr(DMediaServerMngr::instance()),
        srvButton(0),
        srvStatus(0),
        progress(0),
        aStats(0),
        separator(0),
        iStats(0)
    {
    }

    DMediaServerMngr* mngr;
    QPushButton*      srvButton;
    QLabel*           srvStatus;
    WorkingWidget*    progress;
    QLabel*           aStats;
    QLabel*           separator;
    QLabel*           iStats;
};

// --------------------------------------------------------

DMediaServerCtrl::DMediaServerCtrl(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
 
    QGridLayout* const grid = new QGridLayout(this);
    d->srvButton            = new QPushButton(this);
    d->srvStatus            = new QLabel(this);
    d->progress             = new WorkingWidget(this);
    d->aStats               = new QLabel(this);
    d->separator            = new QLabel(QLatin1String(" / "), this);
    d->iStats               = new QLabel(this);

    QLabel* const explanation = new QLabel(this);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt;

    explanation->setText(i18n("The media server permit to share items through the local network "
                              "using <a href='https://en.wikipedia.org/wiki/Digital_Living_Network_Alliance'>DLNA</a> "
                              "standard and <a href='https://en.wikipedia.org/wiki/Universal_Plug_and_Play'>UPNP</a> "
                              "protocol. Many kind of electronic devices can support DLNA, as tablets, cellulars, TV, etc."
                              "<br>Note: depending of the network features and the configuration, "
                              "the delay to discover the server on client devices can take a while."));

    grid->addWidget(d->srvButton, 0, 0, 1, 1);
    grid->addWidget(d->srvStatus, 0, 1, 1, 1);
    grid->addWidget(d->aStats,    0, 2, 1, 1);
    grid->addWidget(d->separator, 0, 3, 1, 1);
    grid->addWidget(d->iStats,    0, 4, 1, 1);
    grid->addWidget(d->progress,  0, 5, 1, 1);
    grid->addWidget(explanation,  1, 0, 1, 6);
    grid->setColumnStretch(1, 10);
    grid->setSpacing(spacing);
    
    // --------------------------------------------------------

    connect(d->srvButton, SIGNAL(clicked()),
            this, SLOT(slotToggleMediaServer()));
}

DMediaServerCtrl::~DMediaServerCtrl()
{
    delete d;
}

void DMediaServerCtrl::updateServerStatus()
{
    if (d->mngr->isRunning())
    {
        d->srvStatus->setText(i18n("Media server is running"));
        d->aStats->setText(i18np("1 album shared", "%1 albums shared", d->mngr->albumsShared()));
        d->separator->setVisible(true);
        d->iStats->setText(i18np("1 item shared",  "%1 items shared",  d->mngr->itemsShared()));
        d->srvButton->setText(i18n("Stop"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));
        d->progress->toggleTimer(true);
        d->progress->setVisible(true);
    }
    else
    {
        d->srvStatus->setText(i18n("Media server is not running"));
        d->aStats->clear();
        d->separator->setVisible(false);
        d->iStats->clear();
        d->srvButton->setText(i18n("Start"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
        d->progress->toggleTimer(false);
        d->progress->setVisible(false);
    }
}

void DMediaServerCtrl::slotToggleMediaServer()
{
    if (!d->mngr->isRunning())
    {
        emit signalStartMediaServer();
    }
    else
    {
        d->mngr->cleanUp();
        updateServerStatus();
    }
}

}  // namespace Digikam

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
        startButton(0),
        stopButton(0),
        srvStatus(0),
        progress(0),
        aStats(0),
        iStats(0)
    {
    }

    DMediaServerMngr* mngr;
    QPushButton*      startButton;
    QPushButton*      stopButton;
    QLabel*           srvStatus;
    WorkingWidget*    progress;
    QLabel*           aStats;
    QLabel*           iStats;
};

// --------------------------------------------------------

DMediaServerCtrl::DMediaServerCtrl(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
 
    QGridLayout* const grid = new QGridLayout(this);
    d->startButton          = new QPushButton(i18n("Start"), this);
    d->srvStatus            = new QLabel(this);
    d->progress             = new WorkingWidget(this);
    d->stopButton           = new QPushButton(i18n("Stop"),  this);
    d->aStats               = new QLabel(this);
    d->iStats               = new QLabel(this);

    grid->addWidget(d->startButton, 0, 0, 1, 1);
    grid->addWidget(d->srvStatus,   0, 1, 1, 1);
    grid->addWidget(d->progress,    0, 2, 1, 1);
    grid->addWidget(d->stopButton,  0, 3, 1, 1);
    grid->addWidget(d->aStats,      1, 0, 1, 4);
    grid->addWidget(d->iStats,      2, 0, 1, 4);
    grid->setColumnStretch(1, 10);
    grid->setSpacing(spacing);
    
    // --------------------------------------------------------

    connect(d->stopButton, SIGNAL(clicked()),
            this, SLOT(slotStopMediaServer()));

    connect(d->startButton, SIGNAL(clicked()),
            this, SIGNAL(signalStartMediaServer()));
}

DMediaServerCtrl::~DMediaServerCtrl()
{
    delete d;
}

void DMediaServerCtrl::updateServerStatus()
{
    QString txt;

    if (d->mngr->isRunning())
    {
        txt = i18n("Media server is running");
        d->aStats->setText(i18np("1 album shared", "%1 albums shared", d->mngr->albumsShared()));
        d->iStats->setText(i18np("1 item shared",  "%1 items shared",  d->mngr->itemsShared()));
        d->startButton->setEnabled(false);
        d->stopButton->setEnabled(true);
        d->progress->toggleTimer(true);
    }
    else
    {
        txt = i18n("Media server is not running");
        d->aStats->clear();
        d->iStats->clear();
        d->startButton->setEnabled(true);
        d->stopButton->setEnabled(false);
        d->progress->toggleTimer(false);
    }

    d->srvStatus->setText(txt);
}

void DMediaServerCtrl::slotStopMediaServer()
{
    d->mngr->cleanUp();
    updateServerStatus();
}

}  // namespace Digikam

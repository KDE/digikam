/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Media Server tab.
 *
 * Copyright (C) 2007-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Ahmed Fathy <ahmed dot fathi dot abdelmageed at gmail dot com>
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

#include "setupdlnaserver.h"

// Qt includes

#include <QPushButton>
#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dbinfoiface.h"
#include "dmediaservermngr.h"
#include "albumselecttabs.h"

namespace Digikam
{

class SetupDlna::Private
{
public:

    Private() :
        mngr(DMediaServerMngr::instance()),
        iface(0),
        startServerOnStartupCheckBox(0),
        startButton(0),
        stopButton(0),
        srvStatus(0),
        aStats(0),
        iStats(0)
    {
    }

    DMediaServerMngr*    mngr;
    DBInfoIface*         iface;
    QCheckBox*           startServerOnStartupCheckBox;
    QPushButton*         startButton;
    QPushButton*         stopButton;
    QLabel*              srvStatus;
    QLabel*              aStats;
    QLabel*              iStats;

    static const QString configGroupName;
    static const QString configStartServerOnStartupCheckBoxEntry;
};

// --------------------------------------------------------

const QString SetupDlna::Private::configGroupName(QLatin1String("DLNA Settings"));
const QString SetupDlna::Private::configStartServerOnStartupCheckBoxEntry(QLatin1String("Start MediaServer At Startup"));

SetupDlna::SetupDlna(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->iface             = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    d->iface->setObjectName(QLatin1String("SetupDlnaIface"));

    const int spacing    = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const dlnaServerSettingsGroup = new QGroupBox(i18n("Share on the Network Options"), panel);
    QVBoxLayout* const gLayout               = new QVBoxLayout(dlnaServerSettingsGroup);


    d->startServerOnStartupCheckBox = new QCheckBox(i18n("Automatic Start Server at Startup"));
    d->startServerOnStartupCheckBox->setWhatsThis(i18n("Set this option to turn-on the DLNA server at digiKam start-up"));
    d->startServerOnStartupCheckBox->setChecked(true);

    DHBox* const btnBox = new DHBox(panel);
    d->startButton      = new QPushButton(i18n("Start"), btnBox);
    d->srvStatus        = new QLabel(btnBox);
    d->stopButton       = new QPushButton(i18n("Stop"),  btnBox);
    btnBox->setStretchFactor(d->srvStatus, 10);
    d->srvStatus->setAlignment(Qt::AlignCenter);

    DHBox* const staBox   = new DHBox(panel);
    d->aStats             = new QLabel(staBox);
    QWidget* const spacer = new QLabel(staBox);
    d->iStats             = new QLabel(staBox);
    d->iStats->setAlignment(Qt::AlignRight);
    staBox->setStretchFactor(spacer, 10);

    gLayout->addWidget(d->startServerOnStartupCheckBox);
    gLayout->addWidget(d->iface->albumChooser(this));
    gLayout->addWidget(btnBox);
    gLayout->addWidget(staBox);
    gLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout->setSpacing(spacing);

    layout->addWidget(dlnaServerSettingsGroup);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->stopButton, SIGNAL(clicked()),
            this, SLOT(slotStopMediaServer()));

    connect(d->startButton, SIGNAL(clicked()),
            this, SLOT(slotStartMediaServer()));

    readSettings();
    updateServerStatus();
}

SetupDlna::~SetupDlna()
{
    delete d;
}

void SetupDlna::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->startServerOnStartupCheckBox->setChecked(group.readEntry(d->configStartServerOnStartupCheckBoxEntry, false));
}

void SetupDlna::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configStartServerOnStartupCheckBoxEntry, d->startServerOnStartupCheckBox->isChecked());
    config->sync();
}

void SetupDlna::slotSelectionChanged()
{
    // TODO
}

void SetupDlna::updateServerStatus()
{
    QString txt;

    if (d->mngr->isRunning())
    {
        txt = i18n("Media server is running");
        d->aStats->setText(i18np("1 album shared", "%1 albums shared", d->mngr->albumsShared()));
        d->iStats->setText(i18np("1 item shared",  "%1 items shared",  d->mngr->itemsShared()));
        d->startButton->setEnabled(false);
        d->stopButton->setEnabled(true);
    }
    else
    {
        txt = i18n("Media server is not running");
        d->aStats->clear();
        d->iStats->clear();
        d->startButton->setEnabled(true);
        d->stopButton->setEnabled(false);
    }

    d->srvStatus->setText(txt);
}

void SetupDlna::slotStartMediaServer()
{
    DInfoInterface::DAlbumIDs albums = d->iface->albumChooserItems();
    MediaServerMap map;

    foreach(int id, albums)
    {
        DAlbumInfo anf(d->iface->albumInfo(id));
        map.insert(anf.title(), d->iface->albumItems(id));
    }

    d->mngr->setCollectionMap(map);
    d->mngr->startMediaServer();
    updateServerStatus();
}

void SetupDlna::slotStopMediaServer()
{
    d->mngr->cleanUp();
    updateServerStatus();
}

}  // namespace Digikam

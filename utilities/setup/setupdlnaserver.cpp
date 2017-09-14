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

#include <QCheckBox>
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
#include "dmediaserverctrl.h"
#include "albumselecttabs.h"

namespace Digikam
{

class SetupDlna::Private
{
public:

    Private() :
        ctrl(0),
        mngr(DMediaServerMngr::instance()),
        iface(0),
        startServerOnStartupCheckBox(0)
    {
    }

    DMediaServerCtrl* ctrl;
    DMediaServerMngr* mngr;
    DBInfoIface*      iface;
    QCheckBox*        startServerOnStartupCheckBox;
};

// --------------------------------------------------------

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
    d->ctrl                         = new DMediaServerCtrl(panel);

    gLayout->addWidget(d->startServerOnStartupCheckBox);
    gLayout->addWidget(d->iface->albumChooser(this));
    gLayout->addWidget(d->ctrl);
    gLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout->setSpacing(spacing);

    layout->addWidget(dlnaServerSettingsGroup);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->ctrl, SIGNAL(signalStartMediaServer()),
            this, SLOT(slotStartMediaServer()));
        
    readSettings();
    d->ctrl->updateServerStatus();
}

SetupDlna::~SetupDlna()
{
    delete d;
}

void SetupDlna::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());

    d->startServerOnStartupCheckBox->setChecked(group.readEntry(d->mngr->configStartServerOnStartupEntry(), false));
}

void SetupDlna::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());
    group.writeEntry(d->mngr->configStartServerOnStartupEntry(), d->startServerOnStartupCheckBox->isChecked());
    config->sync();
}

void SetupDlna::slotSelectionChanged()
{
    // TODO
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
    d->ctrl->updateServerStatus();
}

}  // namespace Digikam

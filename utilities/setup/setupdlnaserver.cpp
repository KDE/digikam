/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup DLNA Server tab.
 *
 * Copyright (C) 2007-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017 by Ahmed Fathy <ahmed dot fathi dot abdelmageed at gmail dot com>
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

#include "fullscreensettings.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class SetupDlna::Private
{
public:

    Private() :
        startServerOnStartupCheckBox(0),
        startInBackgroundCheckBox(0)
    {
    }

    QCheckBox*  startServerOnStartupCheckBox;
    QCheckBox*  startInBackgroundCheckBox;
    static const QString configGroupName;
    static const QString configstartServerOnStartupCheckBoxEntry;
    static const QString configstartInBackgroundCheckBoxEntry;

};


// --------------------------------------------------------
const QString SetupDlna::Private::configGroupName(QLatin1String("DLNA Settings"));
const QString SetupDlna::Private::configstartServerOnStartupCheckBoxEntry(QLatin1String("Start Server On Startup"));
const QString SetupDlna::Private::configstartInBackgroundCheckBoxEntry(QLatin1String("Start Server In Background"));

SetupDlna::SetupDlna(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const dlnaServerSettingsGroup = new QGroupBox(i18n("Server Options"), panel);
    QVBoxLayout* const gLayout             = new QVBoxLayout(dlnaServerSettingsGroup);


    d->startServerOnStartupCheckBox = new QCheckBox(i18n("Automatic Start ?"));
    d->startServerOnStartupCheckBox->setWhatsThis(i18n("Set this option to start the DLNA server on digikam start"));
    gLayout->addWidget(d->startServerOnStartupCheckBox);

    d->startInBackgroundCheckBox = new QCheckBox(i18n("Start In Background ?"));
    d->startInBackgroundCheckBox->setWhatsThis(i18n("Set this option to start the DLNA server "
                                                       "In Background without showing the server window"));
    gLayout->addWidget(d->startInBackgroundCheckBox);


    gLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout->setSpacing(0);


    layout->addWidget(dlnaServerSettingsGroup);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addStretch();

    // --------------------------------------------------------
    connect(d->startServerOnStartupCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotSettingsChanged()));

    readSettings();

    d->startInBackgroundCheckBox->setEnabled(d->startServerOnStartupCheckBox->isChecked());

    // --------------------------------------------------------
}

SetupDlna::~SetupDlna()
{
    delete d;
}

void SetupDlna::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->startServerOnStartupCheckBox->setChecked(group.readEntry(d->configstartServerOnStartupCheckBoxEntry,  false));
    d->startInBackgroundCheckBox->setChecked(group.readEntry(d->configstartInBackgroundCheckBoxEntry,  false));
}

void SetupDlna::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configstartServerOnStartupCheckBoxEntry, d->startServerOnStartupCheckBox->isChecked());
    group.writeEntry(d->configstartInBackgroundCheckBoxEntry, d->startInBackgroundCheckBox->isChecked());
    config->sync();
}

void SetupDlna::slotSettingsChanged()
{
    d->startInBackgroundCheckBox->setEnabled(d->startServerOnStartupCheckBox->isChecked());
}

}  // namespace Digikam

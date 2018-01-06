/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuplighttable.h"

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

class SetupLightTable::Private
{
public:

    Private() :
        autoSyncPreview(0),
        autoLoadOnRightPanel(0),
        clearOnClose(0),
        fullScreenSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configAutoSyncPreviewEntry;
    static const QString configAutoLoadRightPanelEntry;
    static const QString configLoadFullImagesizeEntry;
    static const QString configClearOnCloseEntry;

    QCheckBox*           autoSyncPreview;
    QCheckBox*           autoLoadOnRightPanel;
    QCheckBox*           clearOnClose;

    FullScreenSettings*  fullScreenSettings;
};

const QString SetupLightTable::Private::configGroupName(QLatin1String("LightTable Settings"));
const QString SetupLightTable::Private::configAutoSyncPreviewEntry(QLatin1String("Auto Sync Preview"));
const QString SetupLightTable::Private::configAutoLoadRightPanelEntry(QLatin1String("Auto Load Right Panel"));
const QString SetupLightTable::Private::configClearOnCloseEntry(QLatin1String("Clear On Close"));

// --------------------------------------------------------

SetupLightTable::SetupLightTable(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout* const gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis(i18n("Set this option to automatically synchronize "
                                          "zooming and panning between left and right panels if the "
                                          "images have the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads the image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis(i18n("Set this option to automatically load an image "
                                               "into the right panel when the corresponding item is selected on the thumbbar."));

    d->clearOnClose = new QCheckBox(i18n("Clear the light table on close"));
    d->clearOnClose->setWhatsThis(i18n("Set this option to remove all images "
                                       "from the light table when you close it, "
                                       "or unset it to preserve the images "
                                       "currently on the light table."));

    gLayout->addWidget(d->autoSyncPreview);
    gLayout->addWidget(d->autoLoadOnRightPanel);
    gLayout->addWidget(d->clearOnClose);
    gLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout->setSpacing(0);

    // --------------------------------------------------------

    d->fullScreenSettings = new FullScreenSettings(FS_LIGHTTABLE, panel);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(d->fullScreenSettings);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();

    // --------------------------------------------------------
}

SetupLightTable::~SetupLightTable()
{
    delete d;
}

void SetupLightTable::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    QColor Black(Qt::black);
    QColor White(Qt::white);

    d->fullScreenSettings->readSettings(group);
    d->autoSyncPreview->setChecked(group.readEntry(d->configAutoSyncPreviewEntry,         true));
    d->autoLoadOnRightPanel->setChecked(group.readEntry(d->configAutoLoadRightPanelEntry, true));
    d->clearOnClose->setChecked(group.readEntry(d->configClearOnCloseEntry,               false));
}

void SetupLightTable::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->fullScreenSettings->saveSettings(group);
    group.writeEntry(d->configAutoSyncPreviewEntry,       d->autoSyncPreview->isChecked());
    group.writeEntry(d->configAutoLoadRightPanelEntry,    d->autoLoadOnRightPanel->isChecked());
    group.writeEntry(d->configClearOnCloseEntry,          d->clearOnClose->isChecked());
    config->sync();
}

}  // namespace Digikam

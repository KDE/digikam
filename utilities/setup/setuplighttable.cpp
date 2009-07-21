/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "setuplighttable.moc"

// Qt includes

#include <QColor>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kglobal.h>

namespace Digikam
{

class SetupLightTablePriv
{
public:

    SetupLightTablePriv()
    {
        hideToolBar          = 0;
        autoSyncPreview      = 0;
        autoLoadOnRightPanel = 0;
        loadFullImageSize    = 0;
        clearOnClose         = 0;
    }

    QCheckBox *hideToolBar;
    QCheckBox *autoSyncPreview;
    QCheckBox *autoLoadOnRightPanel;
    QCheckBox *loadFullImageSize;
    QCheckBox *clearOnClose;
};

SetupLightTable::SetupLightTable(QWidget* parent)
               : QScrollArea(parent), d(new SetupLightTablePriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout *gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis(i18n("Set this option to automatically synchronize "
                                          "zooming and panning between left and right panels if the "
                                          "images have the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads the image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis( i18n("Set this option to automatically load an image "
                     "into the right panel when the corresponding item is selected on the thumbbar."));

    d->loadFullImageSize = new QCheckBox(i18n("Load full-sized image"), interfaceOptionsGroup);
    d->loadFullImageSize->setWhatsThis( i18n("Set this option to load the full-sized image "
                     "into the preview panel instead of one at a reduced size. As this option will make it take longer "
                     "to load images, only use it if you have a fast computer."));

    d->hideToolBar  = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);

    d->clearOnClose = new QCheckBox(i18n("Clear the light table on close"));
    d->clearOnClose->setWhatsThis(i18n("Set this option to remove all images "
    		                       "from the light table when you close it, "
    		                       "or unset it to preserve the images "
    		                       "currently on the light table."));

    gLayout->addWidget(d->autoSyncPreview);
    gLayout->addWidget(d->autoLoadOnRightPanel);
    gLayout->addWidget(d->loadFullImageSize);
    gLayout->addWidget(d->hideToolBar);
    gLayout->addWidget(d->clearOnClose);
    gLayout->setMargin(KDialog::spacingHint());
    gLayout->setSpacing(0);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
}

SetupLightTable::~SetupLightTable()
{
    delete d;
}

void SetupLightTable::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("LightTable Settings"));
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->hideToolBar->setChecked(group.readEntry("FullScreen Hide ToolBar",        false));
    d->autoSyncPreview->setChecked(group.readEntry("Auto Sync Preview",          true));
    d->autoLoadOnRightPanel->setChecked(group.readEntry("Auto Load Right Panel", true));
    d->loadFullImageSize->setChecked(group.readEntry("Load Full Image size",     false));
    d->clearOnClose->setChecked(group.readEntry("Clear On Close",                false));
}

void SetupLightTable::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("LightTable Settings"));
    group.writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    group.writeEntry("Auto Sync Preview",       d->autoSyncPreview->isChecked());
    group.writeEntry("Auto Load Right Panel",   d->autoLoadOnRightPanel->isChecked());
    group.writeEntry("Load Full Image size",    d->loadFullImageSize->isChecked());
    group.writeEntry("Clear On Close",          d->clearOnClose->isChecked());
    config->sync();
}

}  // namespace Digikam

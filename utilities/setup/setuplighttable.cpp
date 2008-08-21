/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qlayout.h>
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setuplighttable.h"
#include "setuplighttable.moc"

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
    }

    QCheckBox *hideToolBar;
    QCheckBox *autoSyncPreview;
    QCheckBox *autoLoadOnRightPanel;
    QCheckBox *loadFullImageSize;
};

SetupLightTable::SetupLightTable(QWidget* parent )
               : QWidget(parent)
{
    d = new SetupLightTablePriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"), parent);


    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    QWhatsThis::add( d->autoSyncPreview, i18n("<p>Set this option to automatically synchronize "
                     "zooming and panning between left and right panels if the images have "
                     "the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads image to the right panel"),
                                            interfaceOptionsGroup);
    QWhatsThis::add( d->autoLoadOnRightPanel, i18n("<p>Set this option to automatically load an image "
                     "into the right panel when the corresponding item is selected on the thumbbar."));

    d->loadFullImageSize = new QCheckBox(i18n("Load full image size"), interfaceOptionsGroup);
    QWhatsThis::add( d->loadFullImageSize, i18n("<p>Set this option to load full image size "
                     "into the preview panel instead of a reduced size. Because this option will take more time "
                     "to load images, use it only if you have a fast computer."));

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
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
    KConfig* config = kapp->config();
    QColor Black(Qt::black);
    QColor White(Qt::white);
    config->setGroup("LightTable Settings");
    d->hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));
    d->autoSyncPreview->setChecked(config->readBoolEntry("Auto Sync Preview", true));
    d->autoLoadOnRightPanel->setChecked(config->readBoolEntry("Auto Load Right Panel", true));
    d->loadFullImageSize->setChecked(config->readBoolEntry("Load Full Image size", false));
}

void SetupLightTable::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->writeEntry("Auto Sync Preview", d->autoSyncPreview->isChecked());
    config->writeEntry("Auto Load Right Panel", d->autoLoadOnRightPanel->isChecked());
    config->writeEntry("Load Full Image size", d->loadFullImageSize->isChecked());
    config->sync();
}

}  // namespace Digikam


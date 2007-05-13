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

// QT includes.

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
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klistview.h>
#include <ktrader.h>

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
        hideToolBar     = 0;
        autoSyncPreview = 0;
    }

    QCheckBox *hideToolBar;
    QCheckBox *autoSyncPreview;
};

SetupLightTable::SetupLightTable(QWidget* parent )
               : QWidget(parent)
{
    d = new SetupLightTablePriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"), parent);


    d->autoSyncPreview = new QCheckBox(i18n("Synchronize automaticly Left and "
                                           "Right panels"),
                                      interfaceOptionsGroup);

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                   interfaceOptionsGroup);

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
}

void SetupLightTable::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->writeEntry("Auto Sync Preview", d->autoSyncPreview->isChecked());
    config->sync();
}

}  // namespace Digikam


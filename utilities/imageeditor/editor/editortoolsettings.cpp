/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qhbox.h>
#include <qpushbutton.h>

// KDE includes.

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "editortoolsettings.h"
#include "editortoolsettings.moc"

namespace Digikam
{

class EditorToolSettingsPriv
{

public:

    EditorToolSettingsPriv()
    {
        okBtn      = 0;
        cancelBtn  = 0;
        tryBtn     = 0;
        defaultBtn = 0;
    }

    QPushButton *okBtn;
    QPushButton *cancelBtn;
    QPushButton *tryBtn;
    QPushButton *defaultBtn;
};

EditorToolSettings::EditorToolSettings(QWidget *parent)
                  : QWidget(parent)
{
    d = new EditorToolSettingsPriv;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(this, 3, 2);

    QHBox *btnBox = new QHBox(this);

    d->defaultBtn = new QPushButton(btnBox);
    d->defaultBtn->setText(i18n("Default"));
    d->defaultBtn->setIconSet(SmallIconSet("reload_page"));
    QToolTip::add(d->defaultBtn, i18n("<p>Reset all settings to default values."));

    d->tryBtn = new QPushButton(btnBox);
    d->tryBtn->setText(i18n("Try"));
    d->tryBtn->setIconSet(SmallIconSet("try"));
    QToolTip::add(d->tryBtn, i18n("<p>Try all settings."));

    QLabel *space2 = new QLabel(btnBox);

    d->okBtn = new QPushButton(btnBox);
    d->okBtn->setText(i18n("Ok"));
    d->okBtn->setIconSet(SmallIconSet("ok"));

    d->cancelBtn = new QPushButton(btnBox);
    d->cancelBtn->setText(i18n("Cancel"));
    d->cancelBtn->setIconSet(SmallIconSet("cancel"));

    btnBox->setStretchFactor(space2, 10);

    // ---------------------------------------------------------------

    gridSettings->addMultiCellWidget(btnBox, 1, 1, 0, 1);
    gridSettings->setRowStretch(3, 10);
    gridSettings->setSpacing(KDialog::spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SLOT(signalDefaultClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

} // NameSpace Digikam

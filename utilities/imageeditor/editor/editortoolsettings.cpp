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

// KDE includes.

#include <kpushbutton.h>
#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <khbox.h>

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
        plainPage  = 0;
        btnBox1    = 0;
        btnBox2    = 0;
        user1Btn   = 0;
        user2Btn   = 0;
        user3Btn   = 0;
    }

    KHBox       *btnBox1;
    KHBox       *btnBox2;

    QWidget     *plainPage;

    KPushButton *user1Btn;
    KPushButton *user2Btn;
    KPushButton *user3Btn;

    KPushButton *okBtn;
    KPushButton *cancelBtn;
    KPushButton *tryBtn;
    KPushButton *defaultBtn;
};

EditorToolSettings::EditorToolSettings(int buttonMask, QWidget *parent)
                  : QWidget(parent)
{
    d = new EditorToolSettingsPriv;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(this);

    d->plainPage = new QWidget(this);
    d->btnBox1   = new KHBox(this);
    d->btnBox2   = new KHBox(this);

    // ---------------------------------------------------------------

    d->defaultBtn = new KPushButton(d->btnBox1);
    d->defaultBtn->setGuiItem(KStandardGuiItem::defaults());
    if (!(buttonMask & Default))
        d->defaultBtn->hide();

    d->tryBtn = new KPushButton(d->btnBox1);
    d->tryBtn->setText(i18n("Try"));
    d->tryBtn->setIcon(KIcon(SmallIcon("try")));
    d->tryBtn->setToolTip(i18n("<p>Try all settings."));
    if (!(buttonMask & Try))
        d->tryBtn->hide();

    QLabel *space2 = new QLabel(d->btnBox1);

    d->okBtn = new KPushButton(d->btnBox1);
    d->okBtn->setGuiItem(KStandardGuiItem::ok());
    if (!(buttonMask & Ok))
        d->okBtn->hide();

    d->cancelBtn = new KPushButton(d->btnBox1);
    d->cancelBtn->setGuiItem(KStandardGuiItem::cancel());
    if (!(buttonMask & Cancel))
        d->cancelBtn->hide();

    d->btnBox1->setStretchFactor(space2, 10);

    // ---------------------------------------------------------------

    d->user1Btn = new KPushButton(d->btnBox2);
    if (!(buttonMask & User1))
        d->user1Btn->hide();

    d->user2Btn = new KPushButton(d->btnBox2);
    if (!(buttonMask & User2))
        d->user2Btn->hide();

    d->user3Btn = new KPushButton(d->btnBox2);
    if (!(buttonMask & User3))
        d->user3Btn->hide();

    // ---------------------------------------------------------------

    gridSettings->addWidget(d->plainPage, 0, 0, 1, 2);
    gridSettings->addWidget(d->btnBox2,   1, 0, 1, 2);
    gridSettings->addWidget(d->btnBox1,   2, 0, 1, 2);
    gridSettings->setRowStretch(3, 10);
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));

    connect(d->user1Btn, SIGNAL(clicked()),
            this, SIGNAL(signalUser1Clicked()));

    connect(d->user2Btn, SIGNAL(clicked()),
            this, SIGNAL(signalUser2Clicked()));

    connect(d->user3Btn, SIGNAL(clicked()),
            this, SIGNAL(signalUser3Clicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SLOT(slotDefaultSettings()));
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

int EditorToolSettings::marginHint()
{
    return KDialog::marginHint();
}

int EditorToolSettings::spacingHint()
{
    return KDialog::spacingHint();
}

QWidget *EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

KPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
        return d->defaultBtn;

    if (buttonCode & Try)
        return d->tryBtn;

    if (buttonCode & Ok)
        return d->okBtn;

    if (buttonCode & Cancel)
        return d->cancelBtn;

    if (buttonCode & User1)
        return d->user1Btn;

    if (buttonCode & User2)
        return d->user2Btn;

    if (buttonCode & User3)
        return d->user3Btn;

    return 0;
}

void EditorToolSettings::slotDefaultSettings()
{
}

void EditorToolSettings::setBusy(bool)
{
}

void EditorToolSettings::saveSettings()
{
}

void EditorToolSettings::readSettings()
{
}

} // NameSpace Digikam

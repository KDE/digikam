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

// KDE includes.

#include <kpushbutton.h>
#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>

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
        saveAsBtn  = 0;
        loadBtn    = 0;
    }

    QHBox       *btnBox1;
    QHBox       *btnBox2;

    QWidget     *plainPage;

    KPushButton *okBtn;
    KPushButton *cancelBtn;
    KPushButton *tryBtn;
    KPushButton *defaultBtn;
    KPushButton *saveAsBtn;
    KPushButton *loadBtn;
};

EditorToolSettings::EditorToolSettings(int buttonMask, QWidget *parent)
                  : QWidget(parent)
{
    d = new EditorToolSettingsPriv;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(this, 3, 2);

    d->plainPage = new QWidget(this);
    d->btnBox1   = new QHBox(this);
    d->btnBox2   = new QHBox(this);

    // ---------------------------------------------------------------

    d->defaultBtn = new KPushButton(d->btnBox1);
    d->defaultBtn->setGuiItem(KStdGuiItem::defaults());
    d->defaultBtn->setIconSet(SmallIconSet("reload_page"));
    QToolTip::add(d->defaultBtn, i18n("<p>Reset all settings to their default values."));
    if (!(buttonMask & Default))
        d->defaultBtn->hide();

    d->tryBtn = new KPushButton(d->btnBox1);
    d->tryBtn->setGuiItem(KStdGuiItem::apply());
    d->tryBtn->setText(i18n("Try"));
    QToolTip::add(d->tryBtn, i18n("<p>Try all settings."));
    if (!(buttonMask & Try))
        d->tryBtn->hide();

    QLabel *space = new QLabel(d->btnBox1);

    d->okBtn = new KPushButton(d->btnBox1);
    d->okBtn->setGuiItem(KStdGuiItem::ok());
    if (!(buttonMask & Ok))
        d->okBtn->hide();

    d->cancelBtn = new KPushButton(d->btnBox1);
    d->cancelBtn->setGuiItem(KStdGuiItem::cancel());
    if (!(buttonMask & Cancel))
        d->cancelBtn->hide();

    d->btnBox1->setStretchFactor(space, 10);

    // ---------------------------------------------------------------

    d->loadBtn = new KPushButton(d->btnBox2);
    d->loadBtn->setGuiItem(KStdGuiItem::open());
    QToolTip::add(d->loadBtn, i18n("<p>Load all parameters from settings text file."));
    if (!(buttonMask & Load))
        d->loadBtn->hide();

    QLabel *space2 = new QLabel(d->btnBox2);

    d->saveAsBtn = new KPushButton(d->btnBox2);
    d->saveAsBtn->setGuiItem(KStdGuiItem::saveAs());
    QToolTip::add(d->saveAsBtn, i18n("<p>Save all parameters to settings text file."));
    if (!(buttonMask & SaveAs))
        d->saveAsBtn->hide();

    d->btnBox1->setStretchFactor(space2, 10);

    // ---------------------------------------------------------------

    gridSettings->addMultiCellWidget(d->plainPage, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(d->btnBox2,   1, 1, 0, 1);
    gridSettings->addMultiCellWidget(d->btnBox1,   2, 2, 0, 1);
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

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SLOT(slotDefaultSettings()));

    connect(d->saveAsBtn, SIGNAL(clicked()),
            this, SLOT(slotSaveAs()));

    connect(d->loadBtn, SIGNAL(clicked()),
            this, SLOT(slotLoad()));
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

    if (buttonCode & Load)
        return d->loadBtn;

    if (buttonCode & SaveAs)
        return d->saveAsBtn;

    return 0;
}

} // NameSpace Digikam

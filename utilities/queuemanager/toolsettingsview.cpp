/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : a view to show Batch Tool Settings.
 *
 * Copyright (C) 2008-2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "toolsettingsview.h"
#include "toolsettingsview.moc"

// Qt includes.

#include <QLabel>
#include <QString>
#include <QScrollArea>
#include <QPushButton>

// KDE includes.

#include <kvbox.h>
#include <khbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "themeengine.h"

namespace Digikam
{

class ToolSettingsViewPriv
{

public:

    enum ToolSettingsViewMode
    {
        MessageView=0,
        SettingsView
    };

public:

    ToolSettingsViewPriv()
    {
        messageView       = 0;
        settingsView      = 0;
        settingsViewIcon  = 0;
        settingsViewTitle = 0;
        settingsViewReset = 0;
        tool              = 0;
    }

    QLabel      *messageView;
    QLabel      *settingsViewIcon;
    QLabel      *settingsViewTitle;

    QPushButton *settingsViewReset;

    QScrollArea *settingsView;

    BatchTool   *tool;
};

ToolSettingsView::ToolSettingsView(QWidget *parent)
                : QStackedWidget(parent), d(new ToolSettingsViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------------------------

    d->messageView = new QLabel(this);
    d->messageView->setAlignment(Qt::AlignCenter);

    insertWidget(ToolSettingsViewPriv::MessageView, d->messageView);

    // --------------------------------------------------------------------------

    KVBox *vbox          = new KVBox(this);
    KHBox *hbox          = new KHBox(vbox);
    d->settingsViewIcon  = new QLabel(hbox);
    d->settingsViewTitle = new QLabel(hbox);
    d->settingsViewReset = new QPushButton(hbox);
    d->settingsViewReset->setIcon(SmallIcon("document-revert"));
    d->settingsViewReset->setToolTip(i18n("Reset current tool settings to default values."));

    d->settingsView      = new QScrollArea(vbox);
    d->settingsView->setWidgetResizable(true);
    d->settingsViewIcon->setAlignment(Qt::AlignRight);
    d->settingsViewTitle->setAlignment(Qt::AlignCenter);

    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());
    hbox->setStretchFactor(d->settingsViewTitle, 10);

    vbox->setMargin(0);
    vbox->setSpacing(0);
    vbox->setStretchFactor(d->settingsView, 10);

    insertWidget(ToolSettingsViewPriv::SettingsView, vbox);
    setToolSettingsWidget(new QWidget(this));
    setViewMode(ToolSettingsViewPriv::MessageView);

    // --------------------------------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

ToolSettingsView::~ToolSettingsView()
{
    delete d;
}

void ToolSettingsView::setToolSettingsWidget(QWidget *w)
{
    QWidget *wdt = 0;

    if (!w) wdt = new QWidget;
    else    wdt = w;

    d->settingsView->takeWidget();
    wdt->setParent(d->settingsView->viewport());
    d->settingsView->setWidget(wdt);
    setViewMode(ToolSettingsViewPriv::SettingsView);
}

void ToolSettingsView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->messageView->backgroundRole(), ThemeEngine::instance()->baseColor());
    d->messageView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->settingsView->backgroundRole(), ThemeEngine::instance()->baseColor());
    d->settingsView->setPalette(palette2);
}

int ToolSettingsView::viewMode()
{
    return indexOf(currentWidget());
}

void ToolSettingsView::setViewMode(int mode)
{
    if (mode != ToolSettingsViewPriv::MessageView && mode != ToolSettingsViewPriv::SettingsView)
        return;

    if (mode == ToolSettingsViewPriv::MessageView)
        d->settingsViewReset->setEnabled(false);
    else
        d->settingsViewReset->setEnabled(true);

    setCurrentIndex(mode);
}

void ToolSettingsView::slotToolSelected(const BatchToolSet& set)
{
    if (d->tool)
    {
        disconnect(d->tool, SIGNAL(signalSettingsChanged(const BatchToolSettings&)),
                   this, SLOT(slotSettingsChanged(const BatchToolSettings&)));

        disconnect(d->settingsViewReset, SIGNAL(clicked()),
                   d->tool, SLOT(slotResetSettingsToDefault()));
    }

    d->tool = set.tool;
    if (d->tool)
    {
        d->settingsViewIcon->setPixmap(d->tool->toolIcon().pixmap(QSize(22, 22)));
        d->settingsViewTitle->setText(d->tool->toolTitle());
        d->tool->setSettings(set.settings);
        d->settingsViewReset->setEnabled(true);
        setToolSettingsWidget(d->tool->settingsWidget());

        connect(d->tool, SIGNAL(signalSettingsChanged(const BatchToolSettings&)),
                this, SLOT(slotSettingsChanged(const BatchToolSettings&)));

        connect(d->settingsViewReset, SIGNAL(clicked()),
                d->tool, SLOT(slotResetSettingsToDefault()));
    }
    else
    {
        d->settingsViewIcon->clear();
        d->settingsViewTitle->clear();
        d->settingsViewReset->setEnabled(false);
        setToolSettingsWidget(0);
    }
}

void ToolSettingsView::slotSettingsChanged(const BatchToolSettings& settings)
{
    BatchToolSet set;
    set.tool     = d->tool;
    set.settings = settings;
    emit signalSettingsChanged(set);
}

}  // namespace Digikam

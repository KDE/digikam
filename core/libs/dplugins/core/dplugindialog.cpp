/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin main dialog
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dplugindialog.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>
#include <QPointer>
#include <QPushButton>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dxmlguiwindow.h"
#include "dpluginaboutdlg.h"
#include "dplugin.h"

namespace Digikam
{

DPluginDialog::DPluginDialog(QWidget* const parent, const QString& objName)
    : QDialog(parent),
      m_buttons(0),
      m_tool(0)
{
    setObjectName(objName);
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    m_buttons = new QDialogButtonBox(this);
}

DPluginDialog::~DPluginDialog()
{
    saveDialogSize();
}

void DPluginDialog::setPlugin(DPlugin* const tool)
{
    m_tool = tool;

    if (m_tool)
    {
        QPushButton* const help = m_buttons->addButton(QDialogButtonBox::Help);
        help->setText(i18n("About..."));

        connect(help, SIGNAL(clicked()),
                this, SLOT(slotAboutPlugin()));
    }
}

void DPluginDialog::slotAboutPlugin()
{
    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(m_tool);
    dlg->exec();
    delete dlg;
}

void DPluginDialog::restoreDialogSize()
{
    KConfig config;
    KConfigGroup group = config.group(objectName());

    if (group.exists())
    {
        winId();
        DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
        resize(windowHandle()->size());
    }
    else
    {
        QDesktopWidget* const desktop = QApplication::desktop();
        int screen                    = desktop->screenNumber();
        QRect srect                   = desktop->availableGeometry(screen);
        resize(800 <= srect.width()  ? 800 : srect.width(),
               750 <= srect.height() ? 750 : srect.height());
    }
}

void DPluginDialog::saveDialogSize()
{
    KConfig config;
    KConfigGroup group = config.group(objectName());
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    config.sync();
}

} // namespace Digikam

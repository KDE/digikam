/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard dialog.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dwizarddlg.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>

// KDE includes

#include <kconfig.h>

// Local includes

#include "dxmlguiwindow.h"

namespace Digikam
{

DWizardDlg::DWizardDlg(QWidget* const parent, const QString& objName)
    : QWizard(parent)
{
    setObjectName(objName);
    restoreDialogSize();
}

DWizardDlg::~DWizardDlg()
{
    saveDialogSize();
}

void DWizardDlg::restoreDialogSize()
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

void DWizardDlg::saveDialogSize()
{
    KConfig config;
    KConfigGroup group = config.group(objectName());
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    config.sync();
}

} // namespace Digikam

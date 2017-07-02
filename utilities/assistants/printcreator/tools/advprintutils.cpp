/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-31-01
 * Description : a tool to print images
 *
 * Copyright (C) 2003      by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintutils.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QDir>
#include <QProcess>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

int AdvPrintNint(double n)
{
    return (int)(n + 0.5);
}

// given a list of args, launch this app as a separate thread.
// args[0] is the application to run.
bool AdvPrintLaunchExternalApp(const QString& program, const QStringList& args)
{
    QProcess process;
    process.setProcessEnvironment(adjustedEnvironmentForAppImage());
    return process.startDetached(program, args);
}

bool AdvPrintCheckTempPath(QWidget* const parent, const QString& tempPath)
{
    // does the temp path exist?
    QDir tempDir(tempPath);

    if (!tempDir.exists())
    {
        if (!tempDir.mkdir(tempDir.path()))
        {
            QMessageBox::information(parent, QString(),
                                     i18n("Unable to create a temporary folder. "
                                          "Please make sure you have proper permissions to this folder and try again."));
            return false;
        }
    }

    return true;
}

} // Namespace Digikam

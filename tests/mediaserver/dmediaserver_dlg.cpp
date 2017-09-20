/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for Media Server config dialog.
 *
 * Copyright (C) 2011-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

// Local includes

#include "dmediaserverdlg.h"
#include "dmediaservermngr.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    
    DMediaServerMngr::instance()->load();
    
    DMediaServerDlg* const view = new DMediaServerDlg(&app);
    view->show();
    app.exec();
    
    DMediaServerMngr::instance()->save();
    DMediaServerMngr::instance()->cleanUp();

    return 0;
}

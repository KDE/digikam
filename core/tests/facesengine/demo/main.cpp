/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI test program for FacesEngine
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Alex Jironkin <alexjironkin at gmail dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

// Qt includes

#include <QApplication>

// Local includes

#include "mainwindow.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig(config);
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);

    MainWindow w;
    w.show();
    return a.exec();
}

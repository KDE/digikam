/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : stand alone test application for plugin configuration view.
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

// Qt Includes

#include <QApplication>
#include <QDebug>

// Local includes

#include "dpluginloader.h"
#include "dpluginsetup.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();
    dpl->registerGenericPlugins(qApp);

    DPluginSetup view;
    view.show();
    view.resize(1024, 640);

    app.exec();

    view.applySettings();

    return 0;
}

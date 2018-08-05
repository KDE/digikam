/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : stand alone test application for plugin loader.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QCommandLineParser>
#include <QUrl>
#include <QIcon>
#include <QDebug>

// Local includes

#include "dmetainfoiface.h"
#include "metaengine.h"
#include "dpluginloader.h"
#include "dplugin.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        qDebug() << "loadandrun - Load a plugin and run it";
        qDebug() << "Usage: <plugin name>";
        return -1;
    }

    QString toolName = QString::fromUtf8(argv[1]);
    QApplication app(argc, argv);

    MetaEngine::initializeExiv2();

    bool found = false;
    DPluginLoader dpl;

    foreach (DPlugin* const p, dpl.allPlugins())
    {
        if (p->nameId() == toolName)
        {
            p->init();
            p->slotRun();
            found = true;
            break;
        }
    }

    if (!found)
    {
        qDebug() << toolName << "plugin not found!";
    }

    MetaEngine::cleanupExiv2();

    return 0;
}

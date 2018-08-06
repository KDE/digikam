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
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription(QLatin1String("Test application to run digiKam plugins as stand alone"));

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("list"), QLatin1String("List all available plugins")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("n"),    QLatin1String("Id name of plugin to use"), QLatin1String("String ID")));
    parser.process(app);

    DPluginLoader dpl;

    if (parser.isSet(QString::fromLatin1("list")))
    {
        foreach (DPlugin* const p, dpl.allPlugins())
        {
            qDebug() << "--------------------------------------------";
            qDebug() << "Id     :" << p->id();
            qDebug() << "Name   :" << p->name();
            qDebug() << "Version:" << p->version();
            qDebug() << "Desc   :" << p->description();

            QString authors;

            foreach (const DPluginAuthor& a, p->authors())
            {
                authors.append(a.asString());
                authors.append(QLatin1String(" ; "));
            }

            qDebug() << "Authors:" << authors;
        }

        return 0;
    }
    else if (parser.isSet(QString::fromLatin1("n")) )
    {
        const QString name = parser.value(QString::fromLatin1("n"));

        MetaEngine::initializeExiv2();

        bool found = false;

        foreach (DPlugin* const p, dpl.allPlugins())
        {
            if (p->id() == name)
            {
                p->init();
                p->slotRun();
                found = true;
                break;
            }
        }

        if (!found)
        {
            qDebug() << name << "plugin not found!";
        }

        MetaEngine::cleanupExiv2();
    }
    else
    {
        qDebug() << "Command line option not recognized...";
        qDebug() << "Use --help option for details.";
    }

    return 0;
}

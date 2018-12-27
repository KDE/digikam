/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : stand alone test application for webservice tool.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QPointer>
#include <QDebug>

// Local includes

#include "metaengine.h"
#include "dmetainfoiface.h"
#include "smugwindow.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("import"),
                                        QLatin1String("Import files instead to export")));
    
    QCommandLineOption loginOption(QStringList() << QLatin1String("l") << QLatin1String("login"),
                                   QLatin1String("Login with username (nickname)"),
                                   QLatin1String("nickname"),
                                   QLatin1String("digiKam"));
    parser.addOption(loginOption);
    
    parser.addPositionalArgument(QLatin1String("files"),
                                 QLatin1String("File(s) to open"),
                                 QLatin1String("+[file(s)]"));
    parser.process(app);

    qDebug() << parser.value(loginOption);
    
    MetaEngine::initializeExiv2();

    QList<QUrl> urlList;
    const QStringList args = parser.positionalArguments();

    for (auto& arg : args)
    {
        urlList.append(QUrl::fromLocalFile(arg));
        qDebug() << arg;
    }

    if (parser.isSet(QString::fromLatin1("import")))
    {
        qDebug() << "inside import";
        QPointer<SmugWindow> dlg = new SmugWindow(new DMetaInfoIface(&app, QList<QUrl>()), 0, true);
        dlg->exec();
        delete dlg;
    }
    else if(parser.isSet(QString::fromLatin1("l")))
    {
        qDebug() << "inside login" << parser.value(loginOption);
        QPointer<SmugWindow> dlg = new SmugWindow(new DMetaInfoIface(&app, QList<QUrl>()), 0, false, parser.value(loginOption));
        dlg->exec();
        delete dlg;
    }
    else
    {
        QPointer<SmugWindow> dlg = new SmugWindow(new DMetaInfoIface(&app, QList<QUrl>()), 0);
        dlg->exec();
        delete dlg;
    }

    MetaEngine::cleanupExiv2();

    return 0;
}

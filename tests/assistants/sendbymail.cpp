/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-28
 * Description : stand alone test application for sendbymail tool.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "dmetainfoiface.h"
#include "metaengine.h"
#include "mailwizard.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(QLatin1String("files"), QLatin1String("File(s) to send by mail"), QLatin1String("+[file(s)]"));
    parser.process(app);

    MetaEngine::initializeExiv2();

    QList<QUrl> urlList;
    const QStringList args = parser.positionalArguments();

    for (auto& arg : args)
    {
        urlList.append(QUrl::fromLocalFile(arg));
    }

    MailWizard wzrd(0, new DMetaInfoIface(&app, urlList));
    wzrd.setItemsList(urlList);
    wzrd.exec();

    MetaEngine::cleanupExiv2();

    return 0;
}

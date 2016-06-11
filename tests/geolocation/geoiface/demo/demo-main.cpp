/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  demo-program for GeoIface
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * @author Copyright (C) 2011 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

// KDE includes

#include <klocalizedstring.h>

// local includes

#include "mainwindow.h"
#include "digikam_version.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_single"), i18n("Add built-in demo points as single markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_group"),  i18n("Add built-in demo points as groupable markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("single"),            i18n("Do not group the displayed images")));
    parser.addPositionalArgument(QString::fromLatin1("images"), i18n("List of images"), QString::fromLatin1("[images...]"));
    parser.process(app);

    // get the list of images to load on startup:
    QList<QUrl> imagesList;

    Q_FOREACH(const QString& file, parser.positionalArguments())
    {
        const QUrl argUrl = QUrl::fromLocalFile(file);
        qDebug() << argUrl;
        imagesList << argUrl;
    }

    MainWindow* const myMainWindow = new MainWindow(&parser);
    myMainWindow->show();
    myMainWindow->slotScheduleImagesForLoading(imagesList);

    return app.exec();
}

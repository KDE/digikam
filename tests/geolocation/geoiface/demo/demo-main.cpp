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

#include <kaboutdata.h>
#include <klocalizedstring.h>

// local includes

#include "mainwindow.h"
#include "digikam_version.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData(QString::fromLatin1("demo-geoiface"),
                         i18n("GeoIface demo application"),
                         Digikam::digiKamVersion());
    aboutData.setShortDescription(i18n("Presents the World Map Widget Interface"));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(i18n("(c) 2009-2010 Michael G. Hansen"));
    aboutData.setHomepage(QString::fromLatin1("http://www.digikam.org"));

    aboutData.addAuthor(i18n("Michael G. Hansen"),
                        QString::fromLatin1("mike@mghansen.de"),
                        QString::fromLatin1("http://www.mghansen.de"));

    aboutData.addCredit(i18n("Justus Schwartz"),
                        i18n("Patch for displaying tracks on the map."),
                             QString::fromLatin1("justus at gmx dot li"));

    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_single"), i18n("Add built-in demo points as single markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_group"),  i18n("Add built-in demo points as groupable markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("single"),            i18n("Do not group the displayed images")));
    parser.addPositionalArgument(QString::fromLatin1("images"), i18n("List of images"), QString::fromLatin1("[images...]"));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

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

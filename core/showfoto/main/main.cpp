/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showFoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QDir>
#include <QFile>
#include <QApplication>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QCommandLineOption>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>

// Local includes

#include "metaengine.h"
#include "daboutdata.h"
#include "showfoto.h"
#include "digikam_version.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    // if we have some local breeze icon resource, prefer it
    DXmlGuiWindow::setupIconTheme();

    KLocalizedString::setApplicationDomain("digikam");

    KAboutData aboutData(QString::fromLatin1("showfoto"), // component name
                         i18n("Showfoto"),                // display name
                         digiKamVersion());               // NOTE: showFoto version = digiKam version

    aboutData.setShortDescription(DAboutData::digiKamSlogan());;
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());
    aboutData.setProductName(QByteArray("digikam/showfoto"));   // For bugzilla

    DAboutData::authorsRegistration(aboutData);

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QLatin1String("files"), i18n("File(s) or folder(s) to open"), QLatin1String("[file(s) or folder(s)]"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    QString iconTheme         = group.readEntry(QLatin1String("Icon Theme"), QString());

    MetaEngine::initializeExiv2();

    // Force to use application icon for non plasma desktop as Unity for ex.
    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("showfoto"), app.windowIcon()));

    QList<QUrl> urlList;
    QStringList urls = parser.positionalArguments();

    foreach(const QString& url, urls)
    {
        urlList.append(QUrl::fromLocalFile(url));
    }

    parser.clearPositionalArguments();

    if (!iconTheme.isEmpty())
    {
        QIcon::setThemeName(iconTheme);
    }

    ShowFoto::ShowFoto* const w = new ShowFoto::ShowFoto(urlList);

    // If application storage place in home directory to save customized XML settings files do not exist, create it,
    // else QFile will not able to create new files as well.
    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::DataLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }

    // If application cache place in home directory to save cached files do not exist, create it.
    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    }

    w->show();

    int ret = app.exec();

    MetaEngine::cleanupExiv2();

    return ret;
}

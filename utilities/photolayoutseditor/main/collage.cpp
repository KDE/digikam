/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QApplication>
#include <QIcon>
#include <QCommandLineParser>
#include <QCommandLineOption>

// KDE includes

#include <klocalizedstring.h>
#include <kaboutdata.h>

// Local includes

#include "metaengine.h"
#include "photolayoutswindow.h"
#include "daboutdata.h"
#include "digikam_version.h"

using namespace PhotoLayoutsEditor;
using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    KAboutData aboutData(QString::fromLatin1("photolayoutseditor"), // component name
                         i18n("Photo Layout Editor"),               // display name
                         digiKamVersion());                         // NOTE: photolayouteditor version = digiKam version

    aboutData.setShortDescription(DAboutData::digiKamSlogan());;
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());
    aboutData.setProductName(QByteArray("digikam/photolayoutseditor"));   // For bugzilla

    DAboutData::authorsRegistration(aboutData);


    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QLatin1String("file"), QLatin1String("Template file to open"), QLatin1String("+[file]"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MetaEngine::initializeExiv2();

    QList<QUrl> urlList;
    const QStringList args = parser.positionalArguments();

    for (auto& arg : args)
    {
        urlList.append(QUrl::fromLocalFile(arg));
    }

    parser.clearPositionalArguments();

    PhotoLayoutsWindow* const w = PhotoLayoutsWindow::instance(0);
    w->setAttribute(Qt::WA_DeleteOnClose, true);

    if (!urlList.isEmpty())
    {
        QUrl url = urlList.first();

        if (url.isValid())
            w->open(url);
    }

    w->show();

    int result = app.exec();
    MetaEngine::cleanupExiv2();

    return result;
}

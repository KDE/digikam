/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "welcomepageview.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QWidget>
#include <QApplication>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "daboutdata.h"
#include "thememanager.h"
#include  "webbrowserdlg.h"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* const parent)
    : QWebView(parent)
{
    setFocusPolicy(Qt::WheelFocus);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setRenderHint(QPainter::TextAntialiasing);
    setContextMenuPolicy(Qt::NoContextMenu);

    // ------------------------------------------------------------

    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(slotUrlOpen(const QUrl&)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    QTimer::singleShot(0, this, SLOT(slotThemeChanged()));
}

WelcomePageView::~WelcomePageView()
{
}

void WelcomePageView::slotUrlOpen(const QUrl& url)
{
    WebBrowserDlg* const browser = new WebBrowserDlg(url, this);
    browser->show();
}

QStringList WelcomePageView::featuresTabContent() const
{
    QStringList newFeatures;
    newFeatures << i18n("Port to Qt5 and KF5;");
    newFeatures << i18n("Replacing digiKam KIOSlaves by a multi-threaded interface to query the database;");
    newFeatures << i18n("Replacing Qt5::Multimedia dependency by QtAV framework to handle video files;");
    newFeatures << i18n("Add embedded trash support for each collection instead desktop trash;");
    newFeatures << i18n("Thumbs and preview video support is now delegate to QT5Multimedia framework;");
    newFeatures << i18n("Mysql internal server is now configurable as Sqlite to store database files at a customized place;");
    newFeatures << i18n("Mysql internal/remote server is now configurable with first run assistant;");
    newFeatures << i18n("Add a new garbage collector tool to cleanup database;");
    newFeatures << i18n("Add a new batch queue manager tool to convert RAW files to DNG;");
    newFeatures << i18n("Add a new tool to export contents in html gallery;");
    newFeatures << i18n("Add a new tool to export contents as a video slideshow;");
    newFeatures << i18n("Add a new tool to export contents by email;");
    newFeatures << i18n("Add a new batch queue manager tool to adjust time and date metadata;");
    newFeatures << i18n("Add a new batch queue manager tool to detect and fix red-eyes automatically;");
    newFeatures << i18n("Add a new option in editor and light table to import images from a digital scanner;");
    newFeatures << i18n("Add a new option in editor and light table to edit metadata;");
    newFeatures << i18n("Add a new option in editor and light table to edit geolocation;");
    newFeatures << i18n("Add a new option in editor and light table to run presentation tool;");
    newFeatures << i18n("Add a new editor tool to detect and fix red-eyes automatically;");
    newFeatures << i18n("Add a new editor tool to perform color change based on Lut3D;");
    newFeatures << i18n("Add a new tool in camera import interface to convert RAW files to DNG;");
    newFeatures << i18n("Add a new tool to export items on local network through UPNP/DLNA;");
    newFeatures << i18n("Consolidation of Mysql database backend;");
    newFeatures << i18n("Improved startup time with differed scan for new items stage.");
    newFeatures << i18n("Presentation and Slideshow tools now support video.");
    // Add new features here...
    newFeatures << i18n("...and much more.");

    QString featureItems;

    for (int i = 0 ; i < newFeatures.count() ; ++i)
    {
        featureItems += i18n("<li>%1</li>\n", newFeatures.at(i));
    }

    QString tabHeader = i18n("New Features");
    QString tabContent =
        i18n("<h3>%1</h3><ul>%2</ul>",
             i18n("Some of the new features in this release of digiKam include (compared to digiKam 4.x):"),
             featureItems
            );

    return QStringList() << tabHeader << tabContent;
}

QStringList WelcomePageView::aboutTabContent() const
{
    QString tabHeader = i18n("About");
    QString tabContent =
        i18n("<h3>%1</h3><h3>%2</h3><ul>%3</ul>",
             i18n("digiKam is an open source photo management program designed to import, organize, enhance, search and export your digital images to and from your computer."),
             i18n("Currently, you are in the Album view mode of digiKam. Albums are the places where your files are stored, and are identical to the folders on your hard disk."),
             i18n("<li>%1</li><li>%2</li>",
                  i18n("digiKam has many powerful features which are described in the <a href=\"https://docs.kde.org/trunk5/en/extragear-graphics/digikam/index.html\">documentation</a>"),
                  i18n("The <a href=\"http://www.digikam.org\">digiKam homepage</a> provides information about new versions of digiKam."))
            );
    return QStringList() << tabHeader << tabContent;
}

QByteArray WelcomePageView::fileToString(const QString& aFileName) const
{
    QByteArray   result;
    QFileInfo    info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile        file(aFileName);

    if (aFileName.isEmpty() || len == 0     ||
        !info.exists()      || info.isDir() || !info.isReadable() ||
        !file.open(QIODevice::Unbuffered|QIODevice::ReadOnly))
    {
        return QByteArray();
    }

    result.resize(len + 2);
    readLen = file.read(result.data(), len);

    if (result[len-1] != '\n')
    {
        result[len++] = '\n';
        ++readLen;
    }

    result[len] = '\0';

    if (readLen < len)
    {
        return QByteArray();
    }

    return result;
}

void WelcomePageView::slotThemeChanged()
{
    QString appTitle         = i18n("digiKam");
    QString slogan           = DAboutData::digiKamSlogan();
    QString locationHtml     = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/about/main.html"));

    QString content = QString::fromUtf8(fileToString(locationHtml));
    content         = content.arg(appTitle)
                             .arg(slogan)
                             .arg(i18n("Welcome to digiKam %1", QLatin1String(digikam_version)))
                             .arg(featuresTabContent()[0])
                             .arg(aboutTabContent()[0])
                             .arg(i18n("Background Image Credits"))
                             .arg(featuresTabContent()[1])
                             .arg(aboutTabContent()[1]);

    //qCDebug(DIGIKAM_GENERAL_LOG) << content;

    setHtml(content, QUrl::fromLocalFile(locationHtml));
}

} // namespace Digikam

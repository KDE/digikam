/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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
#include <QDesktopServices>
#include "qstandardpathwrap.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "daboutdata.h"
#include "thememanager.h"
#include "digikam_version.h"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* const parent)
    : QWebView(parent)
{
    setFocusPolicy(Qt::WheelFocus);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setRenderHint(QPainter::TextAntialiasing);

    // ------------------------------------------------------------

    connect(this,SIGNAL(linkClicked(QUrl)),
            this,SLOT(slotUrlOpen(const QUrl&)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    QTimer::singleShot(0, this, SLOT(slotThemeChanged()));
}

WelcomePageView::~WelcomePageView()
{
}

void WelcomePageView::slotUrlOpen(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

QString WelcomePageView::infoPage() const
{
    QStringList newFeatures;
    newFeatures << i18n("Port to Qt5 and KF5;");
    // Add new features here...
    newFeatures << i18n("...and much more.");

    QString featureItems;

    for (int i = 0 ; i < newFeatures.count() ; ++i)
    {
        featureItems += i18n("<li>%1</li>\n", newFeatures.at(i) );
    }

    QString info =
        i18nc(
            "%1: current digiKam version; "
            "%2: digiKam help:// Url; "
            "%3: digiKam homepage Url; "
            "%4: prior digiKam version; "
            "%5: prior KDE version; "
            "%6: generated list of new features; "
            "%7: generated list of important changes; "
            "--- end of comment ---",

            "<h2 style='margin-top: 0px;'>"
            "Welcome to digiKam %1"
            "</h2>"

            "<p>"
            "digiKam is an open source photo management program "
            "designed to import, organize, enhance, search and export your "
            "digital images to and from your computer."
            "</p>"

            "<p>"
            "Currently, you are in the Album view mode of digiKam. Albums are the places "
            "where your files are stored, and are identical to the folders "
            "on your hard disk."
            "</p>"

            "<p>"
            "<ul>"
            "<li>"
            "digiKam has many powerful features which are described in the "
            "<a href=\"%2\">documentation</a>"
            "</li>"
            "<li>"
            "The <a href=\"%3\">digiKam homepage</a> provides information about "
            "new versions of digiKam."
            "</li>"
            "</ul>"
            "</p>"

            "<p>%7</p>"

            "<p>"
            "Some of the new features in this release of digiKam include "
            "(compared to digiKam %4):"
            "</p>"

            "<p>"
            "<ul>%5</ul>"
            "</p>"

            "<p>%6</p>"

            "<p>We hope that you will enjoy digiKam.</p>"

            "<p>Thank you,</p>"

            "<p style='margin-bottom: 0px; margin-left:20px;'>The digiKam team</p>",

            QString(digikam_version),            // %1 : current digiKam version
            "help:/digikam/index.html",          // %2 : digiKam help:// Url
            DAboutData::webProjectUrl().url(),   // %3 : digiKam homepage Url
            "4.x",                               // %4 : prior digiKam version
            featureItems,                        // %5 : prior KDE version
            QString(),                           // %6 : generated list of new features
            QString());                          // %7 : previous digiKam release.

    return info;
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

    if (1 && result[len-1]!='\n')
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
    QString infoPageCss      = QStandardPathsWrap::locate(QStandardPaths::GenericDataLocation, "kf5/infopage/kde_infopage.css");
    QString digikamCss       = QStandardPathsWrap::locate(QStandardPaths::GenericDataLocation, "digikam/about/digikam.css");
    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString slogan           = DAboutData::digiKamSlogan();
    QString locationHtml     = QStandardPathsWrap::locate(QStandardPaths::GenericDataLocation, "digikam/about/main.html");
    QString locationRtl      = QStandardPathsWrap::locate(QStandardPaths::GenericDataLocation, "kf5/infopage/kde_infopage_rtl.css" );
    QString rtl              = qApp->isRightToLeft() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();

    QString content = fileToString(locationHtml);
    content         = content.arg(infoPageCss) // %1
                      .arg(rtl)                // %2
                      .arg(fontSize)           // %3
                      .arg(appTitle)           // %4
                      .arg(slogan)             // %5
                      .arg(infoPage())         // %6
                      .arg(digikamCss);        // %7

    //qCDebug(DIGIKAM_GENERAL_LOG) << content;

    setHtml(content, QUrl::fromLocalFile(locationHtml));
    show();
}

}  // namespace Digikam

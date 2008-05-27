/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QTimer>
#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kapplication.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <ktemporaryfile.h>

// Local includes.

#include "ddebug.h"
#include "version.h"
#include "themeengine.h"
#include "welcomepageview.h"
#include "welcomepageview.moc"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* parent)
               : KHTMLPart(parent)
{
    m_infoPageCssFile = 0;

    widget()->setFocusPolicy(Qt::WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    setPluginsEnabled(false);
    setJScriptEnabled(false); // just make this explicit.
    setJavaEnabled(false);    // just make this explicit.
    setMetaRefreshEnabled(false);
    setURLCursor(Qt::PointingHandCursor);
    view()->adjustSize();

    // ------------------------------------------------------------

    connect(browserExtension(), SIGNAL(openUrlRequest(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
            this, SLOT(slotUrlOpen(const KUrl&)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    QTimer::singleShot(0, this, SLOT(slotThemeChanged()));
}

WelcomePageView::~WelcomePageView()
{
    if (m_infoPageCssFile)
        m_infoPageCssFile->remove();
}

void WelcomePageView::slotUrlOpen(const KUrl& url)
{
   KToolInvocation::invokeBrowser(url.url());
}

QString WelcomePageView::infoPage()
{
    QStringList newFeatures;
    newFeatures << i18n("Designed for KDE4");
    newFeatures << i18n("Hardware handling using KDE4 Solid interface");
    newFeatures << i18n("Multimedia files handling using KDE4 Phonon interface");
    newFeatures << i18n("Geolocation performed using KDE4 Marble interface");
    newFeatures << i18n("XMP metadata support");
    newFeatures << i18n("The database file can be stored on a customized place to support remote album library paths");
    newFeatures << i18n("Support of multiple roots album paths");
    newFeatures << i18n("Thumbnails-bar integration with preview mode and editor for easy navigation between images");
    newFeatures << i18n("Supports the latest camera RAW files");
    newFeatures << i18n("A new camera interface");
    newFeatures << i18n("A new tool to capture photographs from Camera");
    newFeatures << i18n("A new tool based on LensFun library to fix lens defaults automatically");
    newFeatures << i18n("A new tool to perform fuzzy searches based on sketch drawing template");
    newFeatures << i18n("A new tool to perform advanced searches based on photo meta-informations");

    QString featureItems;
    for ( int i = 0 ; i < newFeatures.count() ; i++ )
        featureItems += i18n("<li>%1</li>\n", newFeatures[i] );

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
        "</h2><p>"
        "digiKam is an open source photo management program. "
        "It is designed to import, organize, enhance, search and export your digital photographs on your computer.</p>"
        "<p>You are currently in the Album view mode of digiKam. The Albums are the real "
        "containers where your files are stored, they are identical with the folders "
        "on disk.</p>\n<ul><li>"
        "digiKam has many powerful features which are described in the "
        "<a href=\"%2\">documentation</a></li>\n"
        "<li>The <a href=\"%3\">digiKam homepage</A> provides information about "
        "new versions of digiKam</li></ul>\n"
        "%7\n<p>"
        "Some of the new features in this release of digiKam include "
        "(compared to digiKam %4):</p>\n"
        "<ul>\n%5</ul>\n"
        "%6\n"
        "<p>We hope that you will enjoy digiKam.</p>\n"
        "<p>Thank you,</p>\n"
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam team</p>",

    QString(digikam_version),            // %1 : current digiKam version
    "help:/digikam/index.html",          // %2 : digiKam help:// Url
    "http://www.digikam.org",            // %3 : digiKam homepage Url
    "0.9.4",                             // %4 : prior digiKam version
    featureItems,                        // %5 : prior KDE version
    QString(),                           // %6 : generated list of new features
    QString());                          // %7 : previous digiKam release.

    return info;
}

QByteArray WelcomePageView::fileToString(const QString& aFileName)
{
    QByteArray   result;
    QFileInfo    info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile        file(aFileName);

    if (aFileName.isEmpty() || len <= 0 ||
        !info.exists() || info.isDir() || !info.isReadable() ||
        !file.open(QIODevice::Unbuffered|QIODevice::ReadOnly))
        return QByteArray();

    result.resize(len + 2);
    readLen = file.read(result.data(), len);
    if (1 && result[len-1]!='\n')
    {
        result[len++] = '\n';
        readLen++;
    }
    result[len] = '\0';

    if (readLen < len)
        return QByteArray();

    return result;
}

void WelcomePageView::updateInfoPageCss()
{
    QColor background = ThemeEngine::instance()->baseColor();
    QColor text       = ThemeEngine::instance()->textRegColor();
    QColor highlight  = ThemeEngine::instance()->textSpecialRegColor();

    QString infoPageCss  = fileToString(KStandardDirs::locate("data", "digikam/about/infopage.css"));
    infoPageCss          = infoPageCss
                           .arg(background.name())                                                     // %1
                           .arg(text.name())                                                           // %2
                           .arg(highlight.name())                                                      // %3
                           .arg(highlight.name())                                                      // %4
                           .arg(highlight.name())                                                      // %5
                           .arg(background.name())                                                     // %6
                           .arg(background.name())                                                     // %7
                           .arg(KStandardDirs::locate("data", "digikam/about/top-middle.png"))         // %8
                           .arg(KStandardDirs::locate("data", "digikam/about/top-left-digikam.png"))   // %9
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-left.png"))       // %10
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-right.png"))      // %11
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-middle.png"))     // %12
                           .arg(KStandardDirs::locate("data", "digikam/about/box-middle-left.png"))    // %13
                           .arg(background.name())                                                     // %14
                           .arg(KStandardDirs::locate("data", "digikam/about/box-middle-right.png"))   // %15
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-left.png"))    // %16
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-right.png"))   // %17
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-middle.png")); // %18

    m_infoPageCssFile->open();
    QFile file(m_infoPageCssFile->fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << infoPageCss;
    file.close();
}

void WelcomePageView::slotThemeChanged()
{
    if (m_infoPageCssFile)
        m_infoPageCssFile->remove();

    m_infoPageCssFile = new KTemporaryFile;
    m_infoPageCssFile->setSuffix(".css");
    m_infoPageCssFile->setAutoRemove(false);

    updateInfoPageCss();

    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString slogan           = i18n("Manage your photographs like a professional "
                                    "with the power of open source");
    QString locationHtml     = KStandardDirs::locate("data", "digikam/about/main.html");
    QString locationRtl      = KStandardDirs::locate("data", "digikam/about/infopage_rtl.css" );
    QString rtl              = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();
    QString locationCss      = m_infoPageCssFile->fileName();

    begin(KUrl(locationHtml));

    QString content = fileToString(locationHtml);
    content         = content.arg(locationCss)        // %1
                             .arg(rtl)                // %2
                             .arg(fontSize)           // %3
                             .arg(appTitle)           // %4
                             .arg(slogan)             // %5
                             .arg(infoPage());        // %6

    write(content);
    end();
    show();
}

}  // namespace Digikam

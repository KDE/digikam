/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page 
 *               on root album.
 * 
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QWidget>
#include <QFile>
#include <QFileInfo>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kapplication.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <KToolInvocation>
#include <kglobalsettings.h>

// Local includes.

#include "ddebug.h"
#include "version.h"
#include "welcomepageview.h"
#include "welcomepageview.moc"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* parent)
               : KHTMLPart(parent)
{
    widget()->setFocusPolicy(Qt::WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    setPluginsEnabled(false);
    setJScriptEnabled(false); // just make this explicit.
    setJavaEnabled(false);    // just make this explicit.
    setMetaRefreshEnabled(false);
    setURLCursor(Qt::PointingHandCursor);

    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString catchPhrase      = QString();      // Not enough space for a catch phrase at default window size.
    QString quickDescription = i18n("A Photo-Management Application for KDE");
    QString locationHtml     = KStandardDirs::locate("data", "digikam/about/main.html");
    QString locationCss      = KStandardDirs::locate("data", "digikam/about/kde_infopage.css");
    QString locationRtl      = KStandardDirs::locate("data", "digikam/about/kde_infopage_rtl.css" );
    QString rtl              = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();

    begin(KUrl(locationHtml));

    QString content = fileToString(locationHtml);
    content         = content.arg(locationCss)        // %1
                             .arg(rtl)                // %2
                             .arg(fontSize)           // %3
                             .arg(appTitle)           // %4
                             .arg(catchPhrase)        // %5
                             .arg(quickDescription)   // %6
                             .arg(infoPage());        // %7

    write(content);
    end();
    show();

    connect(browserExtension(), SIGNAL(openUrlRequest(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
            this, SLOT(slotUrlOpen(const KUrl &)));    
}

WelcomePageView::~WelcomePageView()
{
}

void WelcomePageView::slotUrlOpen(const KUrl &url)
{
   KToolInvocation::invokeBrowser(url.url());
}

QString WelcomePageView::infoPage()
{
    QStringList newFeatures;
    newFeatures << i18n("Designed for KDE4");
    newFeatures << i18n("Hardware handling using KDE4 Solid interface");
    newFeatures << i18n("XMP metadata support");
    newFeatures << i18n("A new camera interface");
    newFeatures << i18n("A new tool to capture photographs from Camera");
    newFeatures << i18n("Database file can be stored on a customized place to support remote album library path");
    newFeatures << i18n("Supports of multiple roots album paths");
    newFeatures << i18n("Supports the latest camera RAW files");

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
        "digiKam is a photo management program for the K Desktop Environment. "
        "It is designed to import, organize, and export your digital photographs on your computer."
        "</p>\n<ul><li>"
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
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam Team</p>",
 
    QString(digikam_version),            // %1 : current digiKam version
    "help:/digikam/index.html",          // %2 : digiKam help:// Url
    "http://www.digikam.org",            // %3 : digiKam homepage Url
    "0.9.3",                             // %4 : prior digiKam version
    featureItems,                        // %5 : prior KDE version
    QString(),                           // %6 : generated list of new features
    QString());                          // %7 : previous digiKam release.
    
    return info;
}

QByteArray WelcomePageView::fileToString(const QString &aFileName)
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

}  // namespace Digikam

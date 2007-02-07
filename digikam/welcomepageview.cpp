/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2006-12-20
 * Description : a widget to display a welcome page 
 *               on root album.
 * 
 * Copyright 2006-2007 by Gilles Caulier
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

#include <qwidget.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kapplication.h>
#include <kurl.h>
#include <kstandarddirs.h>

// Local includes.

#include "version.h"
#include "welcomepageview.h"
#include "welcomepageview.moc"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* parent)
               : KHTMLPart(parent)
{
    widget()->setFocusPolicy(QWidget::WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    setPluginsEnabled(false);
    setJScriptEnabled(false); // just make this explicit.
    setJavaEnabled(false);    // just make this explicit.
    setMetaRefreshEnabled(false);
    setURLCursor(KCursor::handCursor());

    QString location = locate("data", "digikam/about/main.html");
    QString content  = fileToString(location);
    content          = content.arg( locate( "data", "digikam/about/kde_infopage.css" ) );
    content          = content.arg( "" );
    
    begin(KURL( location ));
    
    QString fontSize         = QString::number( 12 );
    QString appTitle         = i18n("digiKam");
    QString catchPhrase      = "";
    QString quickDescription = i18n("A Photo-Management Application for KDE");
    write(content.arg(fontSize).arg(appTitle).arg(catchPhrase)
                 .arg(quickDescription).arg(infoPage()));
    end();
    show();

    connect(browserExtension(), SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
            this, SLOT(slotUrlOpen(const KURL &)));    
}

WelcomePageView::~WelcomePageView()
{
}

void WelcomePageView::slotUrlOpen(const KURL &url)
{
    KApplication::kApplication()->invokeBrowser(url.url());
}

QString WelcomePageView::infoPage()
{
    QString info =
        i18n(
        "%1: digiKam version; " 
        "%2: help:// URL; "
        "%3: homepage URL; "
        "%4: prior digiKam version; " 
        "%5: prior KDE version; "
        "%6: generated list of new features; "
        "%7: First-time user text (only shown on first start); "
        "%8: generated list of important changes; "
        "--- end of comment ---",
        "<h2 style='margin-top: 0px;'>Welcome to digiKam %1</h2><p>"
        "digiKam is a photo-management program for the K Desktop Environment. "
        "It is designed to import and organize your digital photograhs on your computer."
        "</p>\n<ul><li>"
        "digiKam has many powerful features which are described in the "
        "<a href=\"%2\">documentation</a></li>\n"
        "<li>The <a href=\"%3\">digiKam homepage</A> offers information about "
        "new versions of digiKam</li></ul>\n"
        "%8\n<p>" // important changes
        "Some of the new features in this release of digiKam include "
        "(compared to digiKam %4):</p>\n"
        "<ul>\n%5</ul>\n"
        "%6\n"
        "<p>We hope that you will enjoy digiKam.</p>\n"
        "<p>Thank you,</p>\n"
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam Team</p>")
        .arg(digikam_version)            // current digiKam version
        .arg("help:/digikam/index.html") // digiKam help:// URL
        .arg("http://www.digikam.org")   // digiKam homepage URL
        .arg("0.8.2");                   // previous digiKam release.
    
    QStringList newFeatures;
    newFeatures << i18n("16 bits/color/pixels image support");
    newFeatures << i18n("Native JPEG-2000 support");
    newFeatures << i18n("Full color management support");
    newFeatures << i18n("Makernote and IPTC metadata support");
    newFeatures << i18n("Geolocalization of photograph");
    newFeatures << i18n("Sidebar used everywhere");
    newFeatures << i18n("Advanced RAW pictures decoding settings");
    newFeatures << i18n("Fast preview of RAW pictures");
    newFeatures << i18n("Metadata support for RAW pictures");
    newFeatures << i18n("New advanced options to download pictures from camera");
    newFeatures << i18n("New advanced options to manage picture Tags");

    QString featureItems;
    for ( uint i = 0 ; i < newFeatures.count() ; i++ )
        featureItems += i18n("<li>%1</li>\n").arg( newFeatures[i] );
    
    info = info.arg( featureItems );
    
    // Add first-time user text (only shown on first start).
    info = info.arg( QString::null ); 

    // Generated list of important changes    
    info = info.arg( QString::null ); 
    
    return info;
}

QCString WelcomePageView::fileToString(const QString &aFileName)
{
    QCString result;
    QFileInfo info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile file(aFileName);
    
    if (aFileName.isEmpty() || len <= 0 || 
        !info.exists() || info.isDir() || !info.isReadable() ||
        !file.open(IO_Raw|IO_ReadOnly)) 
        return QCString();
    
    result.resize(len + 2);
    readLen = file.readBlock(result.data(), len);
    if (1 && result[len-1]!='\n')
    {
        result[len++] = '\n';
        readLen++;
    }
    result[len] = '\0';
    
    if (readLen < len)
        return QCString();
    
    return result;
}

}  // namespace Digikam

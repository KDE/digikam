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

#include <qwidget.h>
#include <qfile.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <Q3CString>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kapplication.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <KToolInvocation>

// Local includes.

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

    QString location = KStandardDirs::locate("data", "digikam/about/main.html");
    QString content  = fileToString(location);
    content          = content.arg( KStandardDirs::locate( "data", "digikam/about/kde_infopage.css" ) );
    content          = content.arg( "" );
    
    begin(KUrl( location ));
    
    QString fontSize         = QString::number( 12 );
    QString appTitle         = i18n("digiKam");
    QString catchPhrase      = "";
    QString quickDescription = i18n("A Photo-Management Application for KDE");
    write(content.arg(fontSize).arg(appTitle).arg(catchPhrase)
                 .arg(quickDescription).arg(infoPage()));
    end();
    show();

    connect(browserExtension(), SIGNAL(openURLRequest(const KUrl &, const KParts::URLArgs &)),
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
    newFeatures << i18n("16-bit/color/pixels image support");
    newFeatures << i18n("Full color management support");
    newFeatures << i18n("Native JPEG-2000 support");
    newFeatures << i18n("Makernote and IPTC metadata support");
    newFeatures << i18n("Photograph geolocalization");
    newFeatures << i18n("Extensive Sidebars");
    newFeatures << i18n("Advanced RAW pictures decoding settings");
    newFeatures << i18n("Fast RAW preview");
    newFeatures << i18n("RAW Metadata support");
    newFeatures << i18n("New advanced camera download options");
    newFeatures << i18n("New advanced picture Tag management");
    newFeatures << i18n("New zooming/panning support in preview mode");
    newFeatures << i18n("New Light Table provides easy comparison for similar pictures");

    QString featureItems;
    for ( int i = 0 ; i < newFeatures.count() ; i++ )
        featureItems += i18n("<li>%1</li>\n", newFeatures[i] );

    QString info =
        i18nc(
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
        "It is designed to import, organize, and export your digital photographs on your computer."
        "</p>\n<ul><li>"
        "digiKam has many powerful features which are described in the "
        "<a href=\"%2\">documentation</a></li>\n"
        "<li>The <a href=\"%3\">digiKam homepage</A> provides information about "
        "new versions of digiKam</li></ul>\n"
        "%8\n<p>" // important changes
        "Some of the new features in this release of digiKam include "
        "(compared to digiKam %4):</p>\n"
        "<ul>\n%5</ul>\n"
        "%6\n"
        "<p>We hope that you will enjoy digiKam.</p>\n"
        "<p>Thank you,</p>\n"
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam Team</p>"
        ,QString(digikam_version)            // current digiKam version
        ,"help:/digikam/index.html" // digiKam help:// URL
        ,"http://www.digikam.org"   // digiKam homepage URL
        ,"0.8.2"
	,featureItems
	,QString()
	,QString(), QString());                   // previous digiKam release.
    
    return info;
}

Q3CString WelcomePageView::fileToString(const QString &aFileName)
{
    Q3CString result;
    QFileInfo info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile file(aFileName);
    
    if (aFileName.isEmpty() || len <= 0 || 
        !info.exists() || info.isDir() || !info.isReadable() ||
        !file.open(QIODevice::Unbuffered|QIODevice::ReadOnly)) 
        return Q3CString();
    
    result.resize(len + 2);
    readLen = file.read(result.data(), len);
    if (1 && result[len-1]!='\n')
    {
        result[len++] = '\n';
        readLen++;
    }
    result[len] = '\0';
    
    if (readLen < len)
        return Q3CString();
    
    return result;
}

}  // namespace Digikam

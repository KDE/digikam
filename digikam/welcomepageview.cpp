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

#include "daboutdata.h"
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

    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString catchPhrase      = QString();      // Not enough space for a catch phrase at default window size.
    QString quickDescription = QString(digiKamDescription());
    QString locationHtml     = locate("data", "digikam/about/main.html");
    QString locationCss      = locate("data", "digikam/about/kde_infopage.css");
    QString locationRtl      = locate("data", "digikam/about/kde_infopage_rtl.css" );
    QString rtl              = kapp->reverseLayout() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();

    begin(KURL(locationHtml));

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
        "digiKam is a photo management program for the K Desktop Environment. "
        "It is designed to import, organize, and export your digital photographs on your computer."
        "</p><p>You are currently in the Album view mode of digiKam. The Albums are the real "
        "containers where your files are stored, they are identical with the folders "
        "on disk.</p>\n<ul><li>"
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
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam Team</p>")
        .arg(digikam_version)            // current digiKam version
        .arg("help:/digikam/index.html") // digiKam help:// URL
        .arg(Digikam::webProjectUrl())   // digiKam homepage URL
        .arg("0.8.2");                   // previous digiKam release.

    QStringList newFeatures;
    newFeatures << i18n("16-bit/color/pixel image support");
    newFeatures << i18n("Full color management support");
    newFeatures << i18n("Native JPEG-2000 support");
    newFeatures << i18n("Makernote and IPTC metadata support");
    newFeatures << i18n("Photograph geolocation");
    newFeatures << i18n("Extensive Sidebars");
    newFeatures << i18n("Advanced RAW image decoding settings");
    newFeatures << i18n("Fast RAW preview");
    newFeatures << i18n("RAW Metadata support");
    newFeatures << i18n("Camera Interface used as generic import tool");
    newFeatures << i18n("New advanced camera download options");
    newFeatures << i18n("New advanced tag management");
    newFeatures << i18n("New zooming/panning support in preview mode");
    newFeatures << i18n("New Light Table provides easy comparison for similar images");
    newFeatures << i18n("New text, mime-type, and rating filters to search contents on icon view");
    newFeatures << i18n("New options to easy navigate between albums, tags and collections");
    newFeatures << i18n("New options to recursively show the contents of sub-folders");
    newFeatures << i18n("New text filter to search contents on folder views");
    newFeatures << i18n("New options to count of items on all folder views");
    newFeatures << i18n("New tool to perform dates search around whole albums collection: Time-Line");
    newFeatures << i18n("New tool to import RAW files in editor with customized decoding settings");

    QString featureItems;
    for ( uint i = 0 ; i < newFeatures.count() ; i++ )
        featureItems += i18n("<li>%1</li>\n").arg( newFeatures[i] );

    info = info.arg( featureItems );

    // Add first-time user text (only shown on first start).
    info = info.arg( QString() );

    // Generated list of important changes
    info = info.arg( QString() );

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

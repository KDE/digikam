/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "welcomepageview.moc"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>
#include <QWidget>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kdebug.h>

// Local includes

#include "daboutdata.h"
#include "themeengine.h"
#include "version.h"

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
    view()->adjustSize();

    // Disable some KHTMLPart actions as they break predefined digiKam actions.
    // We can re-assign the disabled actions later if we ever need to.
    disablePredefinedActions();

    // ------------------------------------------------------------

    connect(browserExtension(), SIGNAL(openUrlRequest(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
            this, SLOT(slotUrlOpen(const KUrl&)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    QTimer::singleShot(0, this, SLOT(slotThemeChanged()));
}

WelcomePageView::~WelcomePageView()
{
}

void WelcomePageView::slotUrlOpen(const KUrl& url)
{
    KToolInvocation::invokeBrowser(url.url());
}

QString WelcomePageView::infoPage() const
{
    QStringList newFeatures;
    newFeatures << i18n("XMP metadata sidecar support;");
    newFeatures << i18n("Reverse geo-coding support;");
    newFeatures << i18n("Image versioning support;");
    newFeatures << i18n("Face detection and recognition support;");
    newFeatures << i18n("Tag keyboard shortcuts support;");
    newFeatures << i18n("Pick Labels support to improve photograph selection;");
    newFeatures << i18n("Color Labels support to improve photograph workflow;");
    newFeatures << i18n("Re-designed Filters view on right sidebar to perform icon-view items filtering;");
    newFeatures << i18n("Support for the latest camera RAW files using <a href=\"http://www.libraw.org\">LibRaw</a>, "
                        "including Foveon-sensor-based cameras, Nikon D7000, Canon 60D, Pentax K5, Olympus E5 and Sony A450 "
                        "(see the Help menu for the full list of RAW cameras supported);");
    newFeatures << i18n("New advanced RAW decoding settings backported from RawTherapee project;");
    newFeatures << i18n("...and much more.");

    QString featureItems;

    for (int i = 0 ; i < newFeatures.count() ; ++i)
    {
        featureItems += i18n("<li>%1</li>\n", newFeatures[i] );
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
            "1.x",                               // %4 : prior digiKam version
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

    if (aFileName.isEmpty() || len <= 0 ||
        !info.exists() || info.isDir() || !info.isReadable() ||
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
    QString infoPageCss      = KStandardDirs::locate("data", "kdeui/about/kde_infopage.css");
    QString digikamCss       = KStandardDirs::locate("data", "digikam/about/digikam.css");
    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString slogan           = DAboutData::digiKamSlogan().toString();
    QString locationHtml     = KStandardDirs::locate("data", "digikam/about/main.html");
    QString locationRtl      = KStandardDirs::locate("data", "kdeui/about/kde_infopage_rtl.css" );
    QString rtl              = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();

    begin(KUrl(locationHtml).toLocalFile());

    QString content = fileToString(locationHtml);
    content         = content.arg(infoPageCss) // %1
                      .arg(rtl)                // %2
                      .arg(fontSize)           // %3
                      .arg(appTitle)           // %4
                      .arg(slogan)             // %5
                      .arg(infoPage())         // %6
                      .arg(digikamCss);        // %7

    //    kDebug() << content;

    write(content);
    end();
    show();
}

void WelcomePageView::disablePredefinedActions()
{
    KAction* findAction = qobject_cast<KAction*>(actionCollection()->action("find"));

    if (findAction)
    {
        findAction->setShortcut(KShortcut());
    }
    else
    {
        kDebug() << "failed to remove the shortcut of khtml's find action";
    }

    KAction* findNextAction = qobject_cast<KAction*>(actionCollection()->action("findNext"));

    if (findNextAction)
    {
        findNextAction->setShortcut(KShortcut());
    }
    else
    {
        kDebug() << "failed to remove the shortcut of khtml's findNext action";
    }

    KAction* findPreviousAction = qobject_cast<KAction*>(actionCollection()->action("findPrevious"));

    if (findPreviousAction)
    {
        findPreviousAction->setShortcut(KShortcut());
    }
    else
    {
        kDebug() << "failed to remove the shortcut of khtml's findPrevious action";
    }

    KAction* selectAllAction = qobject_cast<KAction*>(actionCollection()->action("selectAll"));

    if (selectAllAction)
    {
        selectAllAction->setShortcut(KShortcut());
    }
    else
    {
        kDebug() << "failed to remove the shortcut of khtml's selectAll action";
    }
}

}  // namespace Digikam
